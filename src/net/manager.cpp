#include "manager.h"

#include "bot_info.h"
#include "tracking.h"

using namespace banggame;

void game_manager::on_message(client_handle client, std::string_view msg) {
    try {
        std::visit([&](auto && msg) {
            auto it = m_connections.find(client);
            if (it == m_connections.end()) {
                throw critical_error("CLIENT_NOT_FOUND");
            }
            connection &con = it->second;
            if constexpr (requires { handle_message(FWD(msg), client, con); }) {
                handle_message(FWD(msg), client, con);
            } else if (auto *value = std::get_if<connection_state::connected>(&con)) {
                handle_message(FWD(msg), value->session);
            } else {
                throw critical_error("CLIENT_NOT_VALIDATED");
            }
        }, deserialize_message(msg));
    } catch (const json::json_error &e) {
        logging::warn("Invalid message: {:.{}}\n{}", msg, net::max_message_log_size, e.what());
        kick_client(client, "INVALID_MESSAGE");
    } catch (const critical_error &e) {
        kick_client(client, e.what());
    } catch (const lobby_error &e) {
        send_message(client, server_messages::lobby_error{ e.what() });
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
    poll();

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
                        send_message(client, server_messages::ping{});
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
                logging::push_context(std::format("game {}", lobby.name));
                lobby.m_game->tick();
                logging::pop_context();
                
                while (lobby.m_game->pending_updates()) {
                    auto [target, update, update_time] = lobby.m_game->get_next_update();
                    for (const game_user &user : lobby.connected_users()) {
                        if (target.matches(lobby.m_game->find_player_by_userid(user.user_id))) {
                            send_message(user.session->client, server_messages::game_update{ update });
                        }
                    }
                }

                if (lobby.m_game->is_game_over()) {
                    lobby.state = lobby_state::finished;
                    broadcast_message_no_lobby(lobby.make_lobby_update());
                }
            } catch (const game_error &e) {
                lobby.state = lobby_state::finished;
                add_lobby_chat_message(lobby, nullptr, { 0, "GAME_ERROR", {chat_format_arg::string{e.what()}}, lobby_chat_flag::translated });
                broadcast_message_no_lobby(lobby.make_lobby_update());
            }
        }
        if (lobby.connected_users().empty()) {
            if (--lobby.lifetime <= ticks{0}) {
                broadcast_message_no_lobby(server_messages::lobby_removed{ lobby.lobby_id });
                return true;
            }
        } else {
            lobby.lifetime = lobby_lifetime;
        }
        return false;
    })) {
        tracking::track_lobby_count(m_lobbies.size());
    }

    if (!m_outgoing_messages.empty()) {
        if (m_outgoing_messages.size() == 1) {
            auto &[client, message] = m_outgoing_messages.front();
            push_message(client, std::move(message));
        } else {
            push_messages(std::move(m_outgoing_messages));
        }
        m_outgoing_messages.clear();
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

void game_manager::handle_message(client_messages::connect &&args, client_handle client, connection &con) {
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
    
    send_message(client, server_messages::client_accepted{ session_id });
    for (const auto &[id, lobby] : m_lobbies) {
        send_message(client, lobby.make_lobby_update());
    }

    if (game_lobby *lobby = session->lobby) {
        handle_join_lobby(session, *lobby);
    }
}

void game_manager::handle_message(client_messages::pong &&args, client_handle client, connection &con) {
    if (auto *value = std::get_if<connection_state::connected>(&con)) {
        value->ping_count = 0;
    } else {
        throw critical_error("CLIENT_NOT_VALIDATED");
    }
}

void game_manager::handle_message(client_messages::user_set_name &&args, session_ptr session) {
    session->set_username(std::move(args.username));

    if (game_lobby *lobby = session->lobby) {
        game_user &user = lobby->find_user(session);
        broadcast_message_lobby(*lobby, user.make_user_update());
    }
}

void game_manager::handle_message(client_messages::user_set_propic &&args, session_ptr session) {
    session->set_propic(std::move(args.propic));

    if (game_lobby *lobby = session->lobby) {
        game_user &user = lobby->find_user(session);
        broadcast_message_lobby(*lobby, user.make_user_update());
    }
}

void game_manager::handle_message(client_messages::lobby_make &&args, session_ptr session) {
    if (session->lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    id_type lobby_id = m_lobbies.empty() ? 1 : m_lobbies.rbegin()->first + 1;
    game_lobby &lobby = m_lobbies.try_emplace(lobby_id, lobby_id, args.name, args.options).first->second;
    tracking::track_lobby_count(m_lobbies.size());

    game_user &user = lobby.add_user(session).first;
    user.flags.add(game_user_flag::lobby_owner);

    lobby.state = lobby_state::waiting;
    lobby.password = args.password;

    broadcast_message_no_lobby(lobby.make_lobby_update());

    send_message(session->client, server_messages::lobby_entered{ user.user_id, lobby.lobby_id, lobby.name, lobby.options });
    send_message(session->client, user.make_user_update());

    add_lobby_chat_message(lobby, &user, {
        0, "USER_JOINED_LOBBY", {chat_format_arg::user{ user.user_id }}, lobby_chat_flag::translated
    });
}

void game_manager::handle_message(client_messages::lobby_game_options &&args, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    if (!user.is_lobby_owner()) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    lobby.options = args.options;
    
    for (const game_user &user : lobby.connected_users()) {
        if (user.session != session) {
            send_message(user.session->client, server_messages::lobby_game_options{ args.options });
        }
    }
}

void game_manager::handle_join_lobby(session_ptr session, game_lobby &lobby) {
    auto [new_user, inserted] = lobby.add_user(session);

    send_message(session->client, server_messages::lobby_entered{ new_user.user_id, lobby.lobby_id, lobby.name, lobby.options });
    
    if (new_user.flags.check(game_user_flag::disconnected)) {
        new_user.flags.remove(game_user_flag::disconnected);
        inserted = true;
    }

    if (rn::none_of(lobby.connected_users(), &game_user::is_lobby_owner)) {
        new_user.flags.add(game_user_flag::lobby_owner);
    }

    for (const game_user &user : lobby.users) {
        if (!user.is_disconnected() && user.session != session) {
            send_message(user.session->client, new_user.make_user_update());
        }
        send_message(session->client, user.make_user_update());
    }
    for (const lobby_bot &bot : lobby.bots) {
        send_message(session->client, bot.make_user_update());
    }
    for (const auto &message: lobby.chat_messages) {
        send_message(session->client, server_messages::lobby_chat{ message });
    }
    if (inserted) {
        add_lobby_chat_message(lobby, &new_user, {
            0, "USER_JOINED_LOBBY", {chat_format_arg::user{ new_user.user_id }}, lobby_chat_flag::translated
        });
    }
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        player_ptr target = lobby.m_game->find_player_by_userid(new_user.user_id);
        if (!target) {
            add_user_flag(lobby, new_user, game_user_flag::spectator);
        }
        send_message(session->client, server_messages::game_started{});

        for (game_update &&update : lobby.m_game->get_spectator_join_updates()) {
            send_message(session->client, server_messages::game_update{ lobby.m_game->serialize_update(update) });
        }
        if (target) {
            for (game_update &&update : lobby.m_game->get_rejoin_updates(target)) {
                send_message(session->client, server_messages::game_update{ lobby.m_game->serialize_update(update) });
            }
        }
        for (game_update &&update : lobby.m_game->get_game_log_updates(target)) {
            send_message(session->client, server_messages::game_update{ lobby.m_game->serialize_update(update) });
        }
    }

    broadcast_message_no_lobby(lobby.make_lobby_update());
}

bool game_manager::add_user_flag(game_lobby &lobby, game_user &user, game_user_flag flag) {
    if (!user.flags.check(flag)) {
        user.flags.add(flag);
        broadcast_message_lobby(lobby, user.make_user_update());
        return true;
    }
    return false;
}

bool game_manager::remove_user_flag(game_lobby &lobby, game_user &user, game_user_flag flag) {
    if (user.flags.check(flag)) {
        user.flags.remove(flag);
        broadcast_message_lobby(lobby, user.make_user_update());
        return true;
    }
    return false;
}

void game_manager::handle_message(client_messages::lobby_join &&args, session_ptr session) {
    if (session->lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    auto lobby_it = m_lobbies.find(args.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        throw lobby_error("ERROR_INVALID_LOBBY");
    }

    game_lobby &lobby = lobby_it->second;
    if (!lobby.password.empty() && lobby.password != args.password) {
        throw lobby_error("ERROR_PASSWORD_INCORRECT");
    }

    handle_join_lobby(session, lobby);
}

void game_manager::kick_user_from_lobby(session_ptr session) {
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    add_user_flag(lobby, user, game_user_flag::disconnected);

    if (remove_user_flag(lobby, user, game_user_flag::lobby_owner)) {
        if (auto range = lobby.connected_users()) {
            add_user_flag(lobby, range.front(), game_user_flag::lobby_owner);
        }
    }

    broadcast_message_no_lobby(lobby.make_lobby_update());

    session->lobby = nullptr;
    send_message(session->client, server_messages::lobby_kick{});

    for (const auto &[id, lobby] : m_lobbies) {
        send_message(session->client, lobby.make_lobby_update());
    }

    add_lobby_chat_message(lobby, nullptr, {
        0, "USER_LEFT_LOBBY", {chat_format_arg::user{ lobby.find_user(session).user_id }}, lobby_chat_flag::translated
    });
}

void game_manager::on_connect(client_handle client) {
    m_connections.emplace(client, connection_state::not_validated{});
    tracking::track_client_count(m_connections.size());
}

void game_manager::send_message(client_handle client, const server_message &msg) {
    m_outgoing_messages.emplace_back(client, serialize_message(msg));
}

void game_manager::broadcast_message_no_lobby(const server_message &msg) {
    std::string message = serialize_message(msg);
    for (session_ptr session : m_sessions | rv::values) {
        if (!session->lobby) {
            m_outgoing_messages.emplace_back(session->client, message);
        }
    }
}

void game_manager::broadcast_message_lobby(const game_lobby &lobby, const server_message &msg) {
    std::string message = serialize_message(msg);
    for (const game_user &user : lobby.connected_users()) {
        m_outgoing_messages.emplace_back(user.session->client, message);
    }
}

void game_manager::invalidate_connection(client_handle client) {
    if (auto it = m_connections.find(client); it != m_connections.end()) {
        auto &con = it->second;
        if (auto *connected = std::get_if<connection_state::connected>(&con)) {
            session_ptr session = connected->session;

            session->client.reset();
            if (game_lobby *lobby = session->lobby) {
                game_user &user = lobby->find_user(session);
                broadcast_message_lobby(*lobby, user.make_user_update());
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

void game_manager::handle_message(client_messages::lobby_leave &&args, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    kick_user_from_lobby(session);
}

void game_manager::add_lobby_chat_message(game_lobby &lobby, game_user *is_read_for, server_messages::lobby_chat message) {
    server_messages::lobby_chat with_is_read = message;
    with_is_read.flags.add(lobby_chat_flag::is_read);
    for (const game_user &user : lobby.connected_users()) {
        if (&user == is_read_for) {
            send_message(user.session->client, with_is_read);
        } else {
            send_message(user.session->client, message);
        }
    }
    lobby.chat_messages.emplace_back(std::move(with_is_read));
}

void game_manager::handle_message(client_messages::lobby_chat &&args, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (!args.message.empty()) {
        game_lobby &lobby = *session->lobby;
        game_user &user = lobby.find_user(session);
        if (!user.is_muted()) {
            add_lobby_chat_message(lobby, nullptr, { user.user_id, args.message });
        }
        if (args.message[0] == chat_command::start_char) {
            handle_chat_command(session, args.message.substr(1));
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
    game_user &user = lobby.find_user(session);

    if (command.permissions().check(command_permissions::lobby_owner) && !user.is_lobby_owner()) {
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

    if (command.permissions().check(command_permissions::lobby_in_game) && !lobby.m_game) {
        throw lobby_error("ERROR_LOBBY_NOT_IN_GAME");
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

void game_manager::handle_message(client_messages::lobby_return &&args, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    if (!user.is_lobby_owner()) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state == lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_WAITING");
    }

    broadcast_message_lobby(lobby, server_messages::lobby_entered{ user.user_id, lobby.lobby_id, lobby.name, lobby.options });

    lobby.bots.clear();
    lobby.m_game.reset();
    lobby.state = lobby_state::waiting;

    for (game_user &user : lobby.connected_users()) {
        remove_user_flag(lobby, user, game_user_flag::spectator);
    }

    broadcast_message_no_lobby(lobby.make_lobby_update());
}

void game_manager::handle_message(client_messages::user_spectate &&args, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;
    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }
    game_user &user = lobby.find_user(session);
    if (args.spectator) {
        add_user_flag(lobby, user, game_user_flag::spectator);
    } else {
        remove_user_flag(lobby, user, game_user_flag::spectator);
    }
    broadcast_message_no_lobby(lobby.make_lobby_update());
}

void game_manager::handle_message(client_messages::game_start &&args, session_ptr session) {
    if (!session->lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    if (!user.is_lobby_owner()) {
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
    broadcast_message_no_lobby(lobby.make_lobby_update());

    broadcast_message_lobby(lobby, server_messages::game_started{});

    logging::push_context(std::format("game {}", lobby.name));
    lobby.m_game = std::make_unique<banggame::game>(lobby.options);

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

        broadcast_message_lobby(lobby, bot.make_user_update());
    }

    lobby.m_game->add_players(user_ids);
    lobby.m_game->start_game();
    lobby.m_game->commit_updates();
    logging::pop_context();
}

void game_manager::handle_message(client_messages::game_rejoin &&args, session_ptr session) {
    game_lobby &lobby = *session->lobby;
    game_user &user = lobby.find_user(session);

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (!user.is_spectator()) {
        throw lobby_error("ERROR_USER_NOT_SPECTATOR");
    }

    if (rn::contains(lobby.connected_users(), args.user_id, &game_user::user_id)) {
        throw lobby_error("ERROR_PLAYER_NOT_REJOINABLE");
    }

    player_ptr target = lobby.m_game->find_player_by_userid(args.user_id);
    if (!target) {
        throw lobby_error("ERROR_CANNOT_FIND_PLAYER");
    }
    if (target->is_bot()) {
        if (lobby.options.allow_bot_rejoin) {
            auto it = rn::find(lobby.bots, args.user_id, &lobby_bot::user_id);
            lobby.bots.erase(it);
        } else {
            throw lobby_error("ERROR_CANNOT_REJOIN_ON_BOT");
        }
    }

    remove_user_flag(lobby, user, game_user_flag::spectator);
    target->user_id = user.user_id;

    lobby.m_game->add_update(game_updates::player_add{ target });
    
    for (game_update &&update : lobby.m_game->get_rejoin_updates(target)) {
        send_message(session->client, server_messages::game_update{ lobby.m_game->serialize_update(update) });
    }
    for (game_update &&update : lobby.m_game->get_game_log_updates(target)) {
        send_message(session->client, server_messages::game_update{ lobby.m_game->serialize_update(update) });
    }

    broadcast_message_no_lobby(lobby.make_lobby_update());
}

void game_manager::handle_message(client_messages::game_action &&args, session_ptr session) {
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

    try {
        logging::push_context(std::format("game {}", lobby.name));
        lobby.m_game->handle_game_action(origin, args.action);
        logging::pop_context();
    } catch (const game_error &e) {
        lobby.state = lobby_state::finished;
        add_lobby_chat_message(lobby, nullptr, { 0, "GAME_ERROR", {chat_format_arg::string{e.what()}}, lobby_chat_flag::translated });
        broadcast_message_no_lobby(lobby.make_lobby_update());
    }
}