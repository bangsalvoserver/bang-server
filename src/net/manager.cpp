#include "manager.h"

#include "bot_info.h"
#include "tracking.h"

using namespace banggame;

void game_manager::on_message(client_handle client, std::string_view msg) {
    try {
        auto client_msg = deserialize_message(json::json::parse(msg));
        utils::visit_tagged([&](utils::tag_for<client_message> auto tag, auto && ... args) {
            auto it = m_clients.find(client);
            if (it == m_clients.end()) {
                throw critical_error("CLIENT_NOT_FOUND");
            }
            client_state &state = it->second;
            if constexpr (requires { handle_message(tag, state, args ...); }) {
                handle_message(tag, state, FWD(args) ...);
            } else if (auto *value = std::get_if<client_state::connected>(&state.state)) {
                handle_message(tag, *(value->user), FWD(args) ...);
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
    for (const auto &[client, state] : m_clients) {
        kick_client(client, "SERVER_STOP");
    }
    
    net::wsserver::stop();
}

void game_manager::tick() {
    net::wsserver::tick();

    for (auto &[client, state] : m_clients) {
        std::visit(overloaded{
            [&](client_state::not_validated &value) {
                if (++value.timeout > client_accept_timer) {
                    kick_client(client, "HANDSHAKE_FAIL");
                }
            },
            [&](client_state::connected &value) {
                if (++value.ping_timer > ping_interval) {
                    value.ping_timer = ticks{};
                    if (++value.ping_count >= pings_until_disconnect) {
                        if (value.user->in_lobby) {
                            kick_user_from_lobby(*(value.user));
                        }
                        kick_client(client, "INACTIVITY");
                    } else {
                        send_message<"ping">(client);
                    }
                }
            },
            [](client_state::invalid) {}
        }, state.state);
    }

    if (std::erase_if(m_users, [&](auto &pair) {
        game_user &user = pair.second;
        if (user.client.expired()) {
            if (--user.lifetime <= ticks{0}) {
                if (user.in_lobby) {
                    kick_user_from_lobby(user);
                }
                return true;
            }
        } else {
            user.lifetime = user_lifetime;
        }
        return false;
    })) {
        tracking::track_user_count(m_users.size());
    }

    if (std::erase_if(m_lobbies, [&](auto &pair) {
        lobby &lobby = pair.second;
        if (lobby.state == lobby_state::playing && lobby.m_game) {
            try {
                lobby.m_game->tick();
                
                while (lobby.m_game->pending_updates()) {
                    auto [target, update, update_time] = lobby.m_game->get_next_update();
                    for (const lobby_user &user : lobby.users) {
                        if (target.matches(user.user_id)) {
                            send_message<"game_update">(user.user->client, update);
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

void game_manager::kick_client(client_handle con, const std::string &msg) {
    if (auto it = m_clients.find(con); it != m_clients.end()) {
        it->second.state.emplace<client_state::invalid>();
    }
    net::wsserver::kick_client(con, msg);
}

void game_manager::handle_message(utils::tag<"connect">, client_state &client, const connect_args &args) {
    if (!std::holds_alternative<client_state::not_validated>(client.state)) {
        throw lobby_error("USER_ALREADY_CONNECTED");
    }
    
    id_type session_id = args.session_id;
    if (session_id == 0) {
        int i = 0;
        while (i < m_options.max_session_id_count) {
            session_id = std::uniform_int_distribution<id_type>{1}(session_rng);
            if (m_users.find(session_id) == m_users.end()) {
                break;
            }
        }
        if (i >= m_options.max_session_id_count) {
            // this is astronomically rare
            throw critical_error("CANNOT_GENERATE_SESSION_ID");
        }
    }

    auto &state = client.state.emplace<client_state::connected>();
    
    if (auto it = m_users.find(session_id); it != m_users.end()) {
        state.user = &it->second;
        kick_client(state.user->client, "RECONNECT_WITH_SAME_SESSION_ID");
    } else {
        state.user = &m_users.emplace_hint(it, session_id, session_id)->second;
        tracking::track_user_count(m_users.size());
    }

    state.user->set_username(args.username);
    state.user->set_propic(args.propic);
    state.user->client = client.client;
    
    send_message<"client_accepted">(client.client, session_id);
    for (const auto &[id, lobby] : m_lobbies) {
        send_message<"lobby_update">(client.client, lobby);
    }

    if (state.user->in_lobby) {
        handle_join_lobby(*state.user, *state.user->in_lobby);
    }
}

void game_manager::handle_message(utils::tag<"pong">, client_state &state) {
    if (auto *value = std::get_if<client_state::connected>(&state.state)) {
        value->ping_count = 0;
    } else {
        throw critical_error("CLIENT_NOT_VALIDATED");
    }
}

void game_manager::handle_message(utils::tag<"user_set_name">, game_user &user, const std::string &username) {
    user.set_username(username);

    if (auto *l = user.in_lobby) {
        lobby_user &lu = l->find_user(user);
        broadcast_message_lobby<"lobby_add_user">(*l, lu.user_id, username, lu.team);
    }
}

void game_manager::handle_message(utils::tag<"user_set_propic">, game_user &user, const utils::image_pixels &propic) {
    user.set_propic(propic);

    if (auto *l = user.in_lobby) {
        lobby_user &lu = l->find_user(user);
        broadcast_message_lobby<"lobby_user_propic">(*l, lu.user_id, propic);
    }
}

void game_manager::handle_message(utils::tag<"lobby_make">, game_user &user, const lobby_info &value) {
    if (user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    id_type lobby_id = m_lobbies.empty() ? 1 : m_lobbies.rbegin()->first + 1;
    lobby &lobby = m_lobbies.try_emplace(lobby_id, value, lobby_id).first->second;
    tracking::track_lobby_count(m_lobbies.size());

    int user_id = lobby.add_user(user).user_id;

    lobby.state = lobby_state::waiting;
    broadcast_message<"lobby_update">(lobby);

    send_message<"lobby_entered">(user.client, user_id, lobby.lobby_id, lobby.name, lobby.options);
    send_message<"lobby_add_user">(user.client, user_id, user.username, lobby_team::game_player);
    if (user.propic) {
        send_message<"lobby_user_propic">(user.client, user_id, user.propic);
    }
}

void game_manager::handle_message(utils::tag<"lobby_edit">, game_user &user, const lobby_info &args) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = *user.in_lobby;

    if (lobby.users.front().user != &user) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    lobby.update_lobby_info(args);
    
    for (const lobby_user &lu : lobby.users) {
        if (lu.user != &user) {
            send_message<"lobby_edited">(lu.user->client, args);
        }
    }
}

void game_manager::handle_join_lobby(game_user &user, lobby &lobby) {
    lobby_user &new_user = lobby.add_user(user);

    broadcast_message<"lobby_update">(lobby);

    send_message<"lobby_entered">(user.client, new_user.user_id, lobby.lobby_id, lobby.name, lobby.options);
    for (const lobby_user &lu : lobby.users) {
        if (lu.user != &user) {
            send_message<"lobby_add_user">(lu.user->client, new_user.user_id, user.username, new_user.team);
            if (user.propic) {
                send_message<"lobby_user_propic">(lu.user->client, new_user.user_id, user.propic);
            }
        }
        send_message<"lobby_add_user">(user.client, lu.user_id, lu.user->username, lu.team, lobby_chat_flag::is_read, lu.user->get_disconnect_lifetime());
        if (lu.user->propic) {
            send_message<"lobby_user_propic">(user.client, lu.user_id, lu.user->propic);
        }
    }
    for (auto &bot : lobby.bots) {
        send_message<"lobby_add_user">(user.client, bot.user_id, bot.username, lobby_team::game_player, lobby_chat_flag::is_read);
        send_message<"lobby_user_propic">(user.client, bot.user_id, *bot.propic);
    }
    for (const auto &message: lobby.chat_messages) {
        send_message<"lobby_chat">(user.client, message);
    }
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        player_ptr target = lobby.m_game->find_player_by_userid(new_user.user_id);
        if (!target) {
            set_user_team(user, lobby_team::game_spectator);
        }
        send_message<"game_started">(user.client);

        for (const auto &msg : lobby.m_game->get_spectator_join_updates()) {
            send_message<"game_update">(user.client, msg);
        }
        if (target) {
            for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
                send_message<"game_update">(user.client, msg);
            }
        }
        for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
            send_message<"game_update">(user.client, msg);
        }
    }
}

void game_manager::set_user_team(game_user &user, lobby_team team) {
    if (user.in_lobby) {
        lobby &lobby = *user.in_lobby;
        auto it = rn::find(lobby.users, &user, &lobby_user::user);
        if (it != lobby.users.end()) {
            it->team = team;
            broadcast_message<"lobby_update">(lobby);
            broadcast_message_lobby<"lobby_add_user">(lobby, it->user_id, user.username, team);
        }
    }
}

void game_manager::handle_message(utils::tag<"lobby_join">, game_user &user, const lobby_id_args &value) {
    if (user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        throw lobby_error("ERROR_INVALID_LOBBY");
    }

    handle_join_lobby(user, lobby_it->second);
}

void game_manager::kick_user_from_lobby(game_user &user) {
    auto &lobby = *std::exchange(user.in_lobby, nullptr);

    auto it = rn::find(lobby.users, &user, &lobby_user::user);
    int user_id = it->user_id;
    lobby.users.erase(it);

    broadcast_message<"lobby_update">(lobby);
    broadcast_message_lobby<"lobby_remove_user">(lobby, user_id);
    send_message<"lobby_kick">(user.client);
}

void game_manager::on_connect(client_handle client) {
    m_clients.emplace(client, client);
    tracking::track_client_count(m_clients.size());
}

void game_manager::on_disconnect(client_handle client) {
    if (auto it = m_clients.find(client); it != m_clients.end()) {
        if (auto *connected = std::get_if<client_state::connected>(&it->second.state)) {
            game_user *user = connected->user;

            user->client.reset();
            if (lobby *l = user->in_lobby) {
                lobby_user &lu = l->find_user(*user);
                broadcast_message_lobby<"lobby_add_user">(*l, lu.user_id, user->username, lu.team, lobby_chat_flag::is_read, user->get_disconnect_lifetime());
            }
        }
        m_clients.erase(it);
        tracking::track_client_count(m_clients.size());
    }
}

void game_manager::handle_message(utils::tag<"lobby_leave">, game_user &user) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    kick_user_from_lobby(user);
}

void game_manager::handle_message(utils::tag<"lobby_chat">, game_user &user, const lobby_chat_client_args &value) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (!value.message.empty()) {
        auto &lobby = *user.in_lobby;
        lobby_user &lu = lobby.find_user(user);
        if (!lu.flags.check(lobby_user_flag::muted)) {
            lobby.chat_messages.emplace_back(lu.user_id, user.username, value.message, lobby_chat_flag::is_read);
            broadcast_message_lobby<"lobby_chat">(lobby, lu.user_id, user.username, value.message);
        }
        if (value.message[0] == chat_command::start_char) {
            handle_chat_command(user, value.message.substr(1));
        }
    }
}

void game_manager::handle_chat_command(game_user &user, const std::string &message) {
    size_t space_pos = message.find_first_of(" \t");
    auto cmd_name = std::string_view(message).substr(0, space_pos);
    auto cmd_it = rn::find(chat_command::commands, cmd_name, &string_command_map::value_type::first);
    if (cmd_it == chat_command::commands.end()) {
        throw lobby_error("INVALID_COMMAND_NAME");
    }

    auto &command = cmd_it->second;
    auto &lobby = *user.in_lobby;

    if (command.permissions().check(command_permissions::lobby_owner) && &user != lobby.users.front().user) {
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

    command(this, user, args);
}

void game_manager::handle_message(utils::tag<"lobby_return">, game_user &user) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = *user.in_lobby;
    lobby_user &lu = lobby.find_user(user);

    if (&user != lobby.users.front().user) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state == lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_WAITING");
    }

    broadcast_message_lobby<"lobby_entered">(lobby, lu.user_id, lobby.lobby_id, lobby.name, lobby.options);

    lobby.bots.clear();
    lobby.m_game.reset();
    lobby.state = lobby_state::waiting;

    for (const lobby_user &user : lobby.users) {
        set_user_team(*(user.user), lobby_team::game_player);
    }
}

void game_manager::handle_message(utils::tag<"user_set_team">, game_user &user, lobby_team team) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (user.in_lobby->state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }
    set_user_team(user, team);
}

void game_manager::handle_message(utils::tag<"game_start">, game_user &user) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = *user.in_lobby;

    if (&user != lobby.users.front().user) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    size_t num_players = rn::count(lobby.users, lobby_team::game_player, &lobby_user::team) + lobby.options.num_bots;

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
    for (const lobby_user &lu : lobby.users) {
        if (lu.team == lobby_team::game_player) {
            user_ids.push_back(lu.user_id);
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

        broadcast_message_lobby<"lobby_add_user">(lobby, bot_id, bot.username, lobby_team::game_player, lobby_chat_flag::is_read);
        broadcast_message_lobby<"lobby_user_propic">(lobby, bot_id, *bot.propic);
    }

    lobby.m_game->add_players(user_ids);
    lobby.m_game->start_game(lobby.options);
    lobby.m_game->commit_updates();
}

void game_manager::handle_message(utils::tag<"game_rejoin">, game_user &user, const game_rejoin_args &value) {
    auto &lobby = *user.in_lobby;
    lobby_user &lu = lobby.find_user(user);

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (lu.team != lobby_team::game_spectator) {
        throw lobby_error("ERROR_USER_NOT_SPECTATOR");
    }

    if (rn::contains(lobby.users, value.user_id, &lobby_user::user_id)) {
        throw lobby_error("ERROR_PLAYER_NOT_REJOINABLE");
    }

    player_ptr target = lobby.m_game->find_player_by_userid(value.user_id);
    if (!target) {
        throw lobby_error("ERROR_CANNOT_FIND_PLAYER");
    }
    if (target->is_bot()) {
        throw lobby_error("ERROR_CANNOT_REJOIN_ON_BOT");
    }

    set_user_team(user, lobby_team::game_player);
    target->user_id = lu.user_id;

    lobby.m_game->add_update<"player_add">(target);
    
    for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
        send_message<"game_update">(user.client, msg);
    }
    for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
        send_message<"game_update">(user.client, msg);
    }
}

void game_manager::handle_message(utils::tag<"game_action">, game_user &user, const json::json &value) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = *user.in_lobby;
    lobby_user &lu = lobby.find_user(user);

    if (lobby.state != lobby_state::playing || !lobby.m_game) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (lobby.m_game->is_waiting()) {
        throw lobby_error("ERROR_GAME_STATE_WAITING");
    }

    player_ptr origin = lobby.m_game->find_player_by_userid(lu.user_id);
    if (!origin) {
        throw lobby_error("ERROR_USER_NOT_CONTROLLING_PLAYER");
    }

    lobby.m_game->handle_game_action(origin, value);
}