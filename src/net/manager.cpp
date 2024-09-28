#include "manager.h"

#include "bot_info.h"
#include "tracking.h"

using namespace banggame;

void game_manager::on_message(client_handle client, std::string_view msg) {
    try {
        auto client_msg = deserialize_message(json::json::parse(msg));
        utils::visit_tagged([&](utils::tag_for<client_message> auto tag, auto && ... args) {
            auto it = m_connections.find(client);
            if (it == m_connections.end()) {
                throw critical_error("CLIENT_NOT_FOUND");
            }
            connection &con = it->second;
            if constexpr (requires { handle_message(tag, client, con, args ...); }) {
                handle_message(tag, client, con, FWD(args) ...);
            } else if (auto *value = std::get_if<connection_state::connected>(&con)) {
                handle_message(tag, value->session, FWD(args) ...);
            } else {
                throw critical_error("CLIENT_NOT_VALIDATED");
            }
        }, client_msg);
    } catch (const json::json_error &e) {
        logging::warn("Invalid message: {}\n{}", msg, e.what());
        kick_client(client, "INVALID_MESSAGE");
    } catch (const critical_error &e) {
        kick_client(client, e.what());
    } catch (const lobby_error &e) {
        send_message<"lobby_error">(client, e.what());
    } catch (const std::exception &e) {
        logging::warn("Error in on_message(): {}", e.what());
    }
}

game_manager::game_manager() {
    std::random_device rd;
    session_rng.seed(rd());
}

void game_manager::stop() {
    for (const auto &[client, con] : m_connections) {
        kick_client(client, "SERVER_STOP");
    }
    
    net::wsserver::stop();
}

void game_manager::tick() {
    net::wsserver::tick();

    for (auto &[client, con] : m_connections) {
        std::visit(overloaded{
            [&](connection_state::not_validated &value) {
                if (++value.timeout > client_accept_timer) {
                    kick_client(client, "HANDSHAKE_FAIL");
                }
            },
            [&](connection_state::connected &value) {
                if (++value.ping_timer > ping_interval) {
                    value.ping_timer = ticks{};
                    if (++value.ping_count >= pings_until_disconnect) {
                        kick_client(client, "INACTIVITY");
                    } else {
                        send_message<"ping">(client);
                    }
                }
            },
            [](connection_state::invalid) {}
        }, con);
    }

    if (std::erase_if(m_sessions, [&](auto &pair) {
        game_session &session = pair.second;
        if (session.client.expired()) {
            if (--session.lifetime <= ticks{0}) {
                if (session.lobby) {
                    kick_user_from_lobby(session);
                }
                return true;
            }
        } else {
            session.lifetime = user_lifetime;
        }
        return false;
    })) {
        tracking::track_user_count(m_sessions.size());
    }

    if (std::erase_if(m_lobbies, [&](auto &pair) {
        game_lobby &lobby = pair.second;
        if (lobby.state == lobby_state::playing && lobby.m_game) {
            try {
                lobby.m_game->tick();
                
                while (lobby.m_game->pending_updates()) {
                    auto [target, update, update_time] = lobby.m_game->get_next_update();
                    for (const game_user &user : lobby.users) {
                        if (target.matches(user.user_id)) {
                            send_message<"game_update">(user.session.client, update);
                        }
                    }
                }

                if (lobby.m_game->is_game_over()) {
                    lobby.state = lobby_state::finished;
                    broadcast_message<"lobby_update">(lobby);
                }
            } catch (const std::exception &e) {
                logging::warn("Error in tick(): {}", e.what());
            }
        }
        if (lobby.users.empty()) {
            if (--lobby.lifetime <= ticks{0}) {
                broadcast_message<"lobby_removed">(lobby.lobby_id);
                return true;
            }
        } else {
            lobby.lifetime = lobby_lifetime;
        }
        return false;
    })) {
        tracking::track_lobby_count(m_lobbies.size());
    }
}

static id_type generate_session_id(auto &rng, auto &map, int max_iters) {
    for (int i = 0; i < max_iters; ++i) {
        id_type value = std::uniform_int_distribution<id_type>{1}(rng);
        if (!map.contains(value)) {
            return value;
        }
    }
    // this is astronomically rare
    throw critical_error("CANNOT_GENERATE_SESSION_ID");
}

void game_manager::handle_message(utils::tag<"connect">, client_handle client, connection &con, const connect_args &args) {
    if (!std::holds_alternative<connection_state::not_validated>(con)) {
        throw lobby_error("USER_ALREADY_CONNECTED");
    }

    id_type session_id = args.session_id;
    if (session_id == 0) {
        session_id = generate_session_id(session_rng, m_sessions, m_options.max_session_id_count);
    }

    auto [it, inserted] = m_sessions.try_emplace(session_id);
    game_session &session = it->second;
    if (inserted) {
        tracking::track_user_count(m_sessions.size());
    } else {
        kick_client(session.client, "RECONNECT_WITH_SAME_SESSION_ID");
    }

    session.set_username(args.username);
    session.set_propic(args.propic);
    session.client = client;
    
    con.emplace<connection_state::connected>(session);
    
    send_message<"client_accepted">(client, session_id);
    for (const auto &[id, lobby] : m_lobbies) {
        send_message<"lobby_update">(client, lobby);
    }

    if (game_lobby *lobby = session.lobby) {
        handle_join_lobby(session, *lobby);
    }
}

void game_manager::handle_message(utils::tag<"pong">, client_handle client, connection &con) {
    if (auto *value = std::get_if<connection_state::connected>(&con)) {
        value->ping_count = 0;
    } else {
        throw critical_error("CLIENT_NOT_VALIDATED");
    }
}

void game_manager::handle_message(utils::tag<"user_set_name">, game_session &session, const std::string &username) {
    session.set_username(username);

    if (game_lobby *lobby = session.lobby) {
        game_user &user = lobby->find_user(session);
        broadcast_message_lobby<"lobby_add_user">(*lobby, user.user_id, username, user.team);
    }
}

void game_manager::handle_message(utils::tag<"user_set_propic">, game_session &session, const utils::image_pixels &propic) {
    session.set_propic(propic);

    if (game_lobby *lobby = session.lobby) {
        game_user &user = lobby->find_user(session);
        broadcast_message_lobby<"lobby_user_propic">(*lobby, user.user_id, propic);
    }
}

void game_manager::handle_message(utils::tag<"lobby_make">, game_session &session, const lobby_info &value) {
    if (session.lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    id_type lobby_id = m_lobbies.empty() ? 1 : m_lobbies.rbegin()->first + 1;
    game_lobby &lobby = m_lobbies.try_emplace(lobby_id, value, lobby_id).first->second;
    tracking::track_lobby_count(m_lobbies.size());

    game_user &user = lobby.add_user(session).first;

    lobby.state = lobby_state::waiting;
    broadcast_message<"lobby_update">(lobby);

    send_message<"lobby_entered">(session.client, user.user_id, lobby.lobby_id, lobby.name, lobby.options);
    send_message<"lobby_add_user">(session.client, user.user_id, session.username, lobby_team::game_player);

    add_lobby_chat_message(lobby, &user, {
        0, "", "USER_JOINED_LOBBY", { session.username },
        { lobby_chat_flag::server_message, lobby_chat_flag::translated } }
    );
    
    if (session.propic) {
        send_message<"lobby_user_propic">(session.client, user.user_id, session.propic);
    }
}

void game_manager::handle_message(utils::tag<"lobby_edit">, game_session &session, const lobby_info &args) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session.lobby;

    if (&lobby.users.front().session != &session) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    lobby.update_lobby_info(args);
    
    for (const game_user &user : lobby.users) {
        if (&user.session != &session) {
            send_message<"lobby_edited">(user.session.client, args);
        }
    }
}

void game_manager::handle_join_lobby(game_session &session, game_lobby &lobby) {
    auto [new_user, inserted] = lobby.add_user(session);

    broadcast_message<"lobby_update">(lobby);

    send_message<"lobby_entered">(session.client, new_user.user_id, lobby.lobby_id, lobby.name, lobby.options);
    for (const game_user &user : lobby.users) {
        if (&user.session != &session) {
            send_message<"lobby_add_user">(user.session.client, new_user.user_id, session.username, new_user.team);
            if (session.propic) {
                send_message<"lobby_user_propic">(user.session.client, new_user.user_id, session.propic);
            }
        }
        send_message<"lobby_add_user">(session.client, user.user_id, user.session.username, user.team, user.session.get_disconnect_lifetime());
        if (const auto &propic = user.session.propic) {
            send_message<"lobby_user_propic">(session.client, user.user_id, propic);
        }
    }
    for (auto &bot : lobby.bots) {
        send_message<"lobby_add_user">(session.client, bot.user_id, bot.username, lobby_team::game_player);
        send_message<"lobby_user_propic">(session.client, bot.user_id, *bot.propic);
    }
    for (const auto &message: lobby.chat_messages) {
        send_message<"lobby_chat">(session.client, message);
    }
    if (inserted) {
        add_lobby_chat_message(lobby, &new_user, {
            0, "", "USER_JOINED_LOBBY", { session.username },
            { lobby_chat_flag::server_message, lobby_chat_flag::translated } }
        );
    }
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        player_ptr target = lobby.m_game->find_player_by_userid(new_user.user_id);
        if (!target) {
            set_user_team(session, lobby_team::game_spectator);
        }
        send_message<"game_started">(session.client);

        for (const auto &msg : lobby.m_game->get_spectator_join_updates()) {
            send_message<"game_update">(session.client, msg);
        }
        if (target) {
            for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
                send_message<"game_update">(session.client, msg);
            }
        }
        for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
            send_message<"game_update">(session.client, msg);
        }
    }
}

void game_manager::set_user_team(game_session &session, lobby_team team) {
    if (game_lobby *lobby = session.lobby) {
        game_user &user = lobby->find_user(session);
        user.team = team;
        broadcast_message<"lobby_update">(*lobby);
        broadcast_message_lobby<"lobby_add_user">(*lobby, user.user_id, session.username, team);
    }
}

void game_manager::handle_message(utils::tag<"lobby_join">, game_session &session, const lobby_id_args &value) {
    if (session.lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        throw lobby_error("ERROR_INVALID_LOBBY");
    }

    handle_join_lobby(session, lobby_it->second);
}

void game_manager::kick_user_from_lobby(game_session &session) {
    game_lobby &lobby = *std::exchange(session.lobby, nullptr);

    auto user_id = lobby.remove_user(session).user_id;

    broadcast_message<"lobby_update">(lobby);
    broadcast_message_lobby<"lobby_remove_user">(lobby, user_id);
    send_message<"lobby_kick">(session.client);

    add_lobby_chat_message(lobby, nullptr, {
        0, "", "USER_LEFT_LOBBY", { session.username },
        { lobby_chat_flag::server_message, lobby_chat_flag::translated } }
    );
}

void game_manager::on_connect(client_handle client) {
    m_connections.emplace(client, connection_state::not_validated{});
    tracking::track_client_count(m_connections.size());
}

void game_manager::invalidate_connection(client_handle client) {
    if (auto it = m_connections.find(client); it != m_connections.end()) {
        auto &con = it->second;
        if (auto *connected = std::get_if<connection_state::connected>(&con)) {
            game_session &session = connected->session;

            session.client.reset();
            if (game_lobby *lobby = session.lobby) {
                game_user &user = lobby->find_user(session);
                broadcast_message_lobby<"lobby_add_user">(*lobby, user.user_id, session.username, user.team, session.get_disconnect_lifetime());
            }
        }

        con.emplace<connection_state::invalid>();
    }
}

void game_manager::kick_client(client_handle client, std::string message) {
    invalidate_connection(client);
    net::wsserver::kick_client(client, std::move(message));
}

void game_manager::on_disconnect(client_handle client) {
    invalidate_connection(client);
    m_connections.erase(client);
    tracking::track_client_count(m_connections.size());
}

void game_manager::handle_message(utils::tag<"lobby_leave">, game_session &session) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    kick_user_from_lobby(session);
}

void game_manager::add_lobby_chat_message(game_lobby &lobby, game_user *is_read_for, lobby_chat_args message) {
    lobby_chat_args with_is_read = message;
    with_is_read.flags.add(lobby_chat_flag::is_read);
    for (const game_user &user : lobby.users) {
        if (&user == is_read_for) {
            send_message<"lobby_chat">(user.session.client, with_is_read);
        } else {
            send_message<"lobby_chat">(user.session.client, message);
        }
    }
    lobby.chat_messages.emplace_back(std::move(with_is_read));
}

void game_manager::handle_message(utils::tag<"lobby_chat">, game_session &session, const lobby_chat_client_args &value) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (!value.message.empty()) {
        game_lobby &lobby = *session.lobby;
        game_user &user = lobby.find_user(session);
        if (!user.flags.check(game_user_flag::muted)) {
            add_lobby_chat_message(lobby, nullptr, { user.user_id, session.username, value.message });
        }
        if (value.message[0] == chat_command::start_char) {
            handle_chat_command(session, value.message.substr(1));
        }
    }
}

void game_manager::handle_chat_command(game_session &session, const std::string &message) {
    size_t space_pos = message.find_first_of(" \t");
    auto cmd_name = std::string_view(message).substr(0, space_pos);
    auto cmd_it = rn::find(chat_command::commands, cmd_name, &string_command_map::value_type::first);
    if (cmd_it == chat_command::commands.end()) {
        throw lobby_error("INVALID_COMMAND_NAME");
    }

    auto &command = cmd_it->second;
    game_lobby &lobby = *session.lobby;

    if (command.permissions().check(command_permissions::lobby_owner) && &session != &lobby.users.front().session) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (command.permissions().check(command_permissions::lobby_waiting) && lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    if (command.permissions().check(command_permissions::lobby_playing) && lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (command.permissions().check(command_permissions::lobby_finished) && lobby.state != lobby_state::finished) {
        throw lobby_error("ERROR_LOBBY_NOT_FINISHED");
    }

    if (command.permissions().check(command_permissions::game_cheat)) {
        if (lobby.state != lobby_state::playing) {
            throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
        } else if (!m_options.enable_cheats) {
            throw lobby_error("ERROR_GAME_CHEATS_NOT_ENABLED");
        }
    }

    std::vector<std::string> args;

    if (space_pos != std::string::npos) {
        std::istringstream stream(message.substr(space_pos));
        std::string token;

        while (stream >> std::quoted(token)) {
            args.push_back(token);
        }
    }

    command(this, session, args);
}

void game_manager::handle_message(utils::tag<"lobby_return">, game_session &session) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session.lobby;
    game_user &user = lobby.find_user(session);

    if (&session != &lobby.users.front().session) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state == lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_WAITING");
    }

    broadcast_message_lobby<"lobby_entered">(lobby, user.user_id, lobby.lobby_id, lobby.name, lobby.options);

    lobby.bots.clear();
    lobby.m_game.reset();
    lobby.state = lobby_state::waiting;

    for (const game_user &user : lobby.users) {
        set_user_team(user.session, lobby_team::game_player);
    }
}

void game_manager::handle_message(utils::tag<"user_set_team">, game_session &session, lobby_team team) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (session.lobby->state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }
    set_user_team(session, team);
}

void game_manager::handle_message(utils::tag<"game_start">, game_session &session) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session.lobby;

    if (&session != &lobby.users.front().session) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    size_t num_players = rn::count(lobby.users, lobby_team::game_player, &game_user::team) + lobby.options.num_bots;

    if (num_players < 3) {
        throw lobby_error("ERROR_NOT_ENOUGH_PLAYERS");
    } else if (num_players > lobby_max_players) {
        throw lobby_error("ERROR_TOO_MANY_PLAYERS");
    }

    lobby.state = lobby_state::playing;
    broadcast_message<"lobby_update">(lobby);

    broadcast_message_lobby<"game_started">(lobby);

    lobby.m_game = std::make_unique<banggame::game>(lobby.options.game_seed);

    logging::info("Started game {} with seed {}", lobby.name, lobby.m_game->rng_seed);

    std::vector<int> user_ids;
    for (const game_user &user : lobby.users) {
        if (user.team == lobby_team::game_player) {
            user_ids.push_back(user.user_id);
        }
    }

    auto names = bot_info.names
        | rv::sample(lobby.options.num_bots, lobby.m_game->bot_rng)
        | rn::to<std::vector<std::string_view>>;

    std::vector<const utils::image_pixels *> propics = bot_info.propics
        | rv::addressof
        | rv::sample(lobby.options.num_bots, lobby.m_game->bot_rng)
        | rn::to_vector;

    for (int i=0; i < lobby.options.num_bots; ++i) {
        int bot_id = -1-i;
        auto &bot = lobby.bots.emplace_back(bot_id, std::format("BOT {}", names[i % names.size()]), propics[i % propics.size()]);
        user_ids.push_back(bot_id);

        broadcast_message_lobby<"lobby_add_user">(lobby, bot_id, bot.username, lobby_team::game_player);
        broadcast_message_lobby<"lobby_user_propic">(lobby, bot_id, *bot.propic);
    }

    lobby.m_game->add_players(user_ids);
    lobby.m_game->start_game(lobby.options);
    lobby.m_game->commit_updates();
}

void game_manager::handle_message(utils::tag<"game_rejoin">, game_session &session, const game_rejoin_args &value) {
    game_lobby &lobby = *session.lobby;
    game_user &user = lobby.find_user(session);

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (user.team != lobby_team::game_spectator) {
        throw lobby_error("ERROR_USER_NOT_SPECTATOR");
    }

    if (rn::contains(lobby.users, value.user_id, &game_user::user_id)) {
        throw lobby_error("ERROR_PLAYER_NOT_REJOINABLE");
    }

    player_ptr target = lobby.m_game->find_player_by_userid(value.user_id);
    if (!target) {
        throw lobby_error("ERROR_CANNOT_FIND_PLAYER");
    }
    if (target->is_bot()) {
        throw lobby_error("ERROR_CANNOT_REJOIN_ON_BOT");
    }

    set_user_team(session, lobby_team::game_player);
    target->user_id = user.user_id;

    lobby.m_game->add_update<"player_add">(target);
    
    for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
        send_message<"game_update">(session.client, msg);
    }
    for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
        send_message<"game_update">(session.client, msg);
    }
}

void game_manager::handle_message(utils::tag<"game_action">, game_session &session, const json::json &value) {
    if (!session.lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session.lobby;
    game_user &user = lobby.find_user(session);

    if (lobby.state != lobby_state::playing || !lobby.m_game) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (lobby.m_game->is_waiting()) {
        throw lobby_error("ERROR_GAME_STATE_WAITING");
    }

    player_ptr origin = lobby.m_game->find_player_by_userid(user.user_id);
    if (!origin) {
        throw lobby_error("ERROR_USER_NOT_CONTROLLING_PLAYER");
    }

    lobby.m_game->handle_game_action(origin, value);
}