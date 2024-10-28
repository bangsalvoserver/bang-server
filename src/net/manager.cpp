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
        logging::warn("Invalid message: {:.{}}\n{}", msg, net::max_message_log_size, e.what());
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
        session_ptr session = pair.second;
        if (session->client.expired()) {
            if (--session->lifetime <= ticks{0}) {
                if (session->lobby) {
                    kick_user_from_lobby(session);
                }
                return true;
            }
        } else {
            session->lifetime = user_lifetime;
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
                    for (const game_user &user : lobby.connected_users()) {
                        if (target.matches(user.user_id)) {
                            send_message<"game_update">(user.session->client, update);
                        }
                    }
                }

                if (lobby.m_game->is_game_over()) {
                    lobby.state = lobby_state::finished;
                    broadcast_message_no_lobby<"lobby_update">(lobby);
                }
            } catch (const std::exception &e) {
                logging::warn("Error in tick(): {}", e.what());
            }
        }
        if (lobby.connected_users().empty()) {
            if (--lobby.lifetime <= ticks{0}) {
                broadcast_message_no_lobby<"lobby_removed">(lobby.lobby_id);
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

void game_manager::handle_message(utils::tag<"connect">, client_handle client, connection &con, connect_args args) {
    if (!std::holds_alternative<connection_state::not_validated>(con)) {
        throw lobby_error("USER_ALREADY_CONNECTED");
    }

    id_type session_id = args.session_id;
    if (session_id == 0) {
        session_id = generate_session_id(session_rng, m_sessions, m_options.max_session_id_count);
    }

    auto [it, inserted] = m_sessions.try_emplace(session_id);
    session_ptr &session = it->second;
    if (inserted) {
        session = std::make_shared<game_session>();
        tracking::track_user_count(m_sessions.size());
    } else {
        kick_client(session->client, "RECONNECT_WITH_SAME_SESSION_ID");
    }

    session->set_username(std::move(args.username));
    session->set_propic(std::move(args.propic));
    session->client = client;
    
    con.emplace<connection_state::connected>(session);
    
    send_message<"client_accepted">(client, session_id);
    for (const auto &[id, lobby] : m_lobbies) {
        send_message<"lobby_update">(client, lobby);
    }

    if (game_lobby *lobby = session->lobby) {
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

void game_manager::handle_message(utils::tag<"user_set_name">, session_ptr session, std::string username) {
    session->set_username(std::move(username));

    if (game_lobby *lobby = session->lobby) {
        game_user &user = lobby->find_user(session);
        broadcast_message_lobby<"lobby_user_update">(*lobby, user.user_id, session->username, session->propic, user.flags);
    }
}

void game_manager::handle_message(utils::tag<"user_set_propic">, session_ptr session, image_pixels propic) {
    session->set_propic(std::move(propic));

    if (game_lobby *lobby = session->lobby) {
        game_user &user = lobby->find_user(session);
        broadcast_message_lobby<"lobby_user_update">(*lobby, user.user_id, session->username, session->propic, user.flags);
    }
}

void game_manager::handle_message(utils::tag<"lobby_make">, session_ptr session, const lobby_make_args &value) {
    if (session->lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    id_type lobby_id = m_lobbies.empty() ? 1 : m_lobbies.rbegin()->first + 1;
    game_lobby &lobby = m_lobbies.try_emplace(lobby_id, lobby_id, value.name, value.options).first->second;
    tracking::track_lobby_count(m_lobbies.size());

    game_user &user = lobby.add_user(session).first;

    lobby.state = lobby_state::waiting;
    lobby.password = value.password;

    broadcast_message_no_lobby<"lobby_update">(lobby);

    send_message<"lobby_entered">(session->client, user.user_id, lobby.lobby_id, lobby.name, lobby.options);
    send_message<"lobby_user_update">(session->client, user.user_id, session->username, session->propic);

    add_lobby_chat_message(lobby, &user, {
        0, "USER_JOINED_LOBBY", {{ utils::tag<"user">{}, user.user_id }}, lobby_chat_flag::translated
    });
}

void game_manager::handle_message(utils::tag<"lobby_game_options">, session_ptr session, const game_options &options) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;

    if (!lobby.is_owner(session)) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    lobby.options = options;
    
    for (const game_user &user : lobby.connected_users()) {
        if (user.session != session) {
            send_message<"lobby_game_options">(user.session->client, options);
        }
    }
}

void game_manager::handle_join_lobby(session_ptr session, game_lobby &lobby) {
    auto [new_user, inserted] = lobby.add_user(session);

    send_message<"lobby_entered">(session->client, new_user.user_id, lobby.lobby_id, lobby.name, lobby.options);
    
    if (remove_user_flag(session, game_user_flag::disconnected)) {
        inserted = true;
    }
    broadcast_message_no_lobby<"lobby_update">(lobby);

    for (const game_user &user : lobby.users) {
        if (!user.is_disconnected() && user.session != session) {
            send_message<"lobby_user_update">(user.session->client, new_user.user_id, session->username, session->propic, new_user.flags);
        }
        std::chrono::milliseconds lifetime{};
        if (!user.is_disconnected() && user.session->client.expired()) {
            lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(user.session->lifetime);
        }
        send_message<"lobby_user_update">(session->client, user.user_id, user.session->username, user.session->propic, user.flags, lifetime);
    }
    for (auto &bot : lobby.bots) {
        send_message<"lobby_user_update">(session->client, bot.user_id, bot.username, bot.propic);
    }
    for (const auto &message: lobby.chat_messages) {
        send_message<"lobby_chat">(session->client, message);
    }
    if (inserted) {
        add_lobby_chat_message(lobby, &new_user, {
            0, "USER_JOINED_LOBBY", {{ utils::tag<"user">{}, new_user.user_id }}, lobby_chat_flag::translated
        });
    }
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        player_ptr target = lobby.m_game->find_player_by_userid(new_user.user_id);
        if (!target) {
            add_user_flag(session, game_user_flag::spectator);
        }
        send_message<"game_started">(session->client);

        for (const auto &msg : lobby.m_game->get_spectator_join_updates()) {
            send_message<"game_update">(session->client, msg);
        }
        if (target) {
            for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
                send_message<"game_update">(session->client, msg);
            }
        }
        for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
            send_message<"game_update">(session->client, msg);
        }
    }
}

bool game_manager::add_user_flag(session_ptr session, game_user_flag flag) {
    if (game_lobby *lobby = session->lobby) {
        game_user &user = lobby->find_user(session);
        if (!user.flags.check(flag)) {
            user.flags.add(flag);
            broadcast_message_no_lobby<"lobby_update">(*lobby);
            broadcast_message_lobby<"lobby_user_update">(*lobby, user.user_id, session->username, session->propic, user.flags);
            return true;
        }
    }
    return false;
}

bool game_manager::remove_user_flag(session_ptr session, game_user_flag flag) {
    if (game_lobby *lobby = session->lobby) {
        game_user &user = lobby->find_user(session);
        if (user.flags.check(flag)) {
            user.flags.remove(flag);
            broadcast_message_no_lobby<"lobby_update">(*lobby);
            broadcast_message_lobby<"lobby_user_update">(*lobby, user.user_id, session->username, session->propic, user.flags);
            return true;
        }
    }
    return false;
}

void game_manager::handle_message(utils::tag<"lobby_join">, session_ptr session, const lobby_join_args &value) {
    if (session->lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        throw lobby_error("ERROR_INVALID_LOBBY");
    }

    game_lobby &lobby = lobby_it->second;
    if (!lobby.password.empty() && lobby.password != value.password) {
        throw lobby_error("ERROR_PASSWORD_INCORRECT");
    }

    handle_join_lobby(session, lobby);
}

void game_manager::kick_user_from_lobby(session_ptr session) {
    add_user_flag(session, game_user_flag::disconnected);
    game_lobby &lobby = *std::exchange(session->lobby, nullptr);

    send_message<"lobby_kick">(session->client);
    for (const auto &[id, lobby] : m_lobbies) {
        send_message<"lobby_update">(session->client, lobby);
    }

    add_lobby_chat_message(lobby, nullptr, {
        0, "USER_LEFT_LOBBY", {{ utils::tag<"user">{}, lobby.find_user(session).user_id }}, lobby_chat_flag::translated
    });
}

void game_manager::on_connect(client_handle client) {
    m_connections.emplace(client, connection_state::not_validated{});
    tracking::track_client_count(m_connections.size());
}

void game_manager::invalidate_connection(client_handle client) {
    if (auto it = m_connections.find(client); it != m_connections.end()) {
        auto &con = it->second;
        if (auto *connected = std::get_if<connection_state::connected>(&con)) {
            session_ptr session = connected->session;

            session->client.reset();
            if (game_lobby *lobby = session->lobby) {
                game_user &user = lobby->find_user(session);
                broadcast_message_lobby<"lobby_user_update">(*lobby, user.user_id, session->username, session->propic, user.flags,
                    std::chrono::duration_cast<std::chrono::milliseconds>(user.session->lifetime)
                );
            }
        }

        con.emplace<connection_state::invalid>();
    }
}

void game_manager::kick_client(client_handle client, std::string message, int code) {
    invalidate_connection(client);
    net::wsserver::kick_client(client, std::move(message), code);
}

void game_manager::on_disconnect(client_handle client) {
    invalidate_connection(client);
    m_connections.erase(client);
    tracking::track_client_count(m_connections.size());
}

void game_manager::handle_message(utils::tag<"lobby_leave">, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    kick_user_from_lobby(session);
}

void game_manager::add_lobby_chat_message(game_lobby &lobby, game_user *is_read_for, lobby_chat_args message) {
    lobby_chat_args with_is_read = message;
    with_is_read.flags.add(lobby_chat_flag::is_read);
    for (const game_user &user : lobby.connected_users()) {
        if (&user == is_read_for) {
            send_message<"lobby_chat">(user.session->client, with_is_read);
        } else {
            send_message<"lobby_chat">(user.session->client, message);
        }
    }
    lobby.chat_messages.emplace_back(std::move(with_is_read));
}

void game_manager::handle_message(utils::tag<"lobby_chat">, session_ptr session, const lobby_chat_client_args &value) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (!value.message.empty()) {
        game_lobby &lobby = *session->lobby;
        game_user &user = lobby.find_user(session);
        if (!user.is_muted()) {
            add_lobby_chat_message(lobby, nullptr, { user.user_id, value.message });
        }
        if (value.message[0] == chat_command::start_char) {
            handle_chat_command(session, value.message.substr(1));
        }
    }
}

void game_manager::handle_chat_command(session_ptr session, const std::string &message) {
    size_t space_pos = message.find_first_of(" \t");
    auto cmd_name = std::string_view(message).substr(0, space_pos);
    auto cmd_it = rn::find(chat_command::commands, cmd_name, &string_command_map::value_type::first);
    if (cmd_it == chat_command::commands.end()) {
        throw lobby_error("INVALID_COMMAND_NAME");
    }

    auto &command = cmd_it->second;
    game_lobby &lobby = *session->lobby;

    if (command.permissions().check(command_permissions::lobby_owner) && !lobby.is_owner(session)) {
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

void game_manager::handle_message(utils::tag<"lobby_return">, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    if (!lobby.is_owner(session)) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state == lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_WAITING");
    }

    broadcast_message_lobby<"lobby_entered">(lobby, user.user_id, lobby.lobby_id, lobby.name, lobby.options);

    lobby.bots.clear();
    lobby.m_game.reset();
    lobby.state = lobby_state::waiting;

    for (const game_user &user : lobby.connected_users()) {
        remove_user_flag(user.session, game_user_flag::spectator);
    }

    broadcast_message_no_lobby<"lobby_update">(lobby);
}

void game_manager::handle_message(utils::tag<"user_spectate">, session_ptr session, bool spectator) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (session->lobby->state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }
    if (spectator) {
        add_user_flag(session, game_user_flag::spectator);
    } else {
        remove_user_flag(session, game_user_flag::spectator);
    }
}

void game_manager::handle_message(utils::tag<"game_start">, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;

    if (!lobby.is_owner(session)) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    size_t num_players = rn::count_if(lobby.connected_users(), std::not_fn(&game_user::is_spectator)) + lobby.options.num_bots;

    if (num_players < 3) {
        throw lobby_error("ERROR_NOT_ENOUGH_PLAYERS");
    } else if (num_players > lobby_max_players) {
        throw lobby_error("ERROR_TOO_MANY_PLAYERS");
    }

    lobby.state = lobby_state::playing;
    broadcast_message_no_lobby<"lobby_update">(lobby);

    broadcast_message_lobby<"game_started">(lobby);

    lobby.m_game = std::make_unique<banggame::game>(lobby.options);

    logging::info("Started game {} with seed {}", lobby.name, lobby.m_game->rng_seed);

    std::vector<int> user_ids;
    for (const game_user &user : lobby.connected_users()) {
        if (!user.is_spectator()) {
            user_ids.push_back(user.user_id);
        }
    }

    auto names = bot_info.names
        | rv::sample(lobby.options.num_bots, lobby.m_game->bot_rng)
        | rn::to<std::vector<std::string_view>>;

    auto propics = bot_info.propics
        | rv::sample(lobby.options.num_bots, lobby.m_game->bot_rng)
        | rn::to<std::vector<image_pixels_hash>>;

    for (int i=0; i < lobby.options.num_bots; ++i) {
        int bot_id = -1-i;
        auto &bot = lobby.bots.emplace_back(bot_id, std::format("BOT {}", names[i % names.size()]), propics[i % propics.size()]);
        user_ids.push_back(bot_id);

        broadcast_message_lobby<"lobby_user_update">(lobby, bot_id, bot.username, bot.propic);
    }

    lobby.m_game->add_players(user_ids);
    lobby.m_game->start_game();
    lobby.m_game->commit_updates();
}

void game_manager::handle_message(utils::tag<"game_rejoin">, session_ptr session, const game_rejoin_args &value) {
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (!user.is_spectator()) {
        throw lobby_error("ERROR_USER_NOT_SPECTATOR");
    }

    if (rn::contains(lobby.connected_users(), value.user_id, &game_user::user_id)) {
        throw lobby_error("ERROR_PLAYER_NOT_REJOINABLE");
    }

    player_ptr target = lobby.m_game->find_player_by_userid(value.user_id);
    if (!target) {
        throw lobby_error("ERROR_CANNOT_FIND_PLAYER");
    }
    if (target->is_bot()) {
        throw lobby_error("ERROR_CANNOT_REJOIN_ON_BOT");
    }

    remove_user_flag(session, game_user_flag::spectator);
    target->user_id = user.user_id;

    lobby.m_game->add_update<"player_add">(target);
    
    for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
        send_message<"game_update">(session->client, msg);
    }
    for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
        send_message<"game_update">(session->client, msg);
    }
}

void game_manager::handle_message(utils::tag<"game_action">, session_ptr session, const json::json &value) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;
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