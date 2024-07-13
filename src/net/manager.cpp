#include "manager.h"

#include <iostream>
#include <stdexcept>

#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>

#include "git_version.h"
#include "bot_info.h"

using namespace banggame;

void game_manager::on_message(client_handle client, const std::string &msg) {
    try {
        auto client_msg = json::deserialize<client_message>(json::json::parse(msg));
        if (m_options.verbose) {
            fmt::print("{}: Received {}\n", get_client_ip(client), msg);
            fflush(stdout);
        }
        utils::visit_tagged([&](utils::tag_for<client_message> auto tag, auto && ... args) {
            auto it = m_clients.find(client);
            if (it == m_clients.end()) {
                throw critical_error("CLIENT_NOT_FOUND");
            }
            client_state &state = it->second;
            if constexpr (requires { handle_message(tag, state, args ...); }) {
                handle_message(tag, state, FWD(args) ...);
            } else if (!state.user) {
                throw critical_error("CLIENT_NOT_VALIDATED");
            } else {
                handle_message(tag, *state.user, FWD(args) ...);
            }
        }, client_msg);
    } catch (const critical_error &e) {
        kick_client(client, e.what());
    } catch (const lobby_error &e) {
        send_message<"lobby_error">(client, e.what());
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error in on_message(): {}\n", e.what());
    }
}

game_manager::game_manager() {
    std::random_device rd;
    session_rng.seed(rd());
}

void game_manager::tick() {
    net::wsserver::tick();

    for (auto it = m_clients.begin(); it != m_clients.end(); ) {
        auto &[client, state] = *it;
        ++it;
        if (state.user) {
            if (++state.ping_timer > ping_interval) {
                state.ping_timer = ticks{};
                if (++state.ping_count >= pings_until_disconnect) {
                    if (state.user->in_lobby) {
                        kick_user_from_lobby(*(state.user));
                    }
                    kick_client(client, "INACTIVITY");
                } else {
                    send_message<"ping">(client);
                }
            }
        } else {
            if (++state.ping_timer > client_accept_timer) {
                kick_client(client, "HANDSHAKE_FAIL");
            }
        }
    }

    for (auto it = m_users.begin(); it != m_users.end(); ) {
        game_user &user = it->second;
        if (user.client.expired()) {
            if (--user.lifetime <= ticks{0}) {
                if (user.in_lobby) {
                    kick_user_from_lobby(user);
                }
                it = m_users.erase(it);
                continue;
            }
        } else {
            user.lifetime = user_lifetime;
        }
        ++it;
    }

    for (auto it = m_lobby_order.begin(); it != m_lobby_order.end(); ) {
        lobby &l = (*it)->second;
        if (l.state == lobby_state::playing && l.m_game) {
            try {
                l.m_game->tick();
                
                while (l.state == lobby_state::playing && l.m_game && l.m_game->pending_updates()) {
                    auto [target, update, update_time] = l.m_game->get_next_update();
                    for (auto &[team, user_id, u] : l.users) {
                        if (target.matches(user_id)) {
                            send_message<"game_update">(u->client, update);
                        }
                    }
                }

                if (l.m_game->is_game_over()) {
                    l.state = lobby_state::finished;
                    broadcast_message<"lobby_update">(l);
                }
            } catch (const std::exception &e) {
                fmt::print(stderr, "Error in tick(): {}\n", e.what());
            }
        }
        if (l.users.empty()) {
            --l.lifetime;
        } else {
            l.lifetime = lobby_lifetime;
        }
        if (l.lifetime <= ticks{0}) {
            broadcast_message<"lobby_removed">(l.lobby_id);
            m_lobbies.erase(*it);
            it = m_lobby_order.erase(it);
            continue;
        }
        ++it;
    }
}

void game_manager::handle_message(MSG_TAG(connect), client_state &state, const connect_args &args) {
    if (!net::validate_commit_hash(args.commit_hash)) {
        throw critical_error("INVALID_CLIENT_COMMIT_HASH");
    }
    if (state.user) {
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
    
    if (auto it = m_users.find(session_id); it != m_users.end()) {
        state.user = &it->second;
        kick_client(state.user->client, "RECONNECT_WITH_SAME_SESSION_ID");
        static_cast<user_info &>(*state.user) = args.user;
    } else {
        state.user = &m_users.emplace_hint(it, std::piecewise_construct,
            std::make_tuple(session_id),
            std::make_tuple(args.user, session_id)
        )->second;
    }

    state.user->client = state.client;
    state.ping_timer = ticks{};
    
    send_message<"client_accepted">(state.client, session_id);
    for (const auto it : m_lobby_order) {
        send_message<"lobby_update">(state.client, it->second);
    }

    if (state.user->in_lobby) {
        handle_join_lobby(*state.user, *state.user->in_lobby);
    }
}

void game_manager::handle_message(MSG_TAG(pong), client_state &state) {
    state.ping_count = 0;
}

void game_manager::handle_message(MSG_TAG(user_edit), game_user &user, const user_info &args) {
    static_cast<user_info &>(user) = args;

    if (auto *l = user.in_lobby) {
        broadcast_message_lobby<"lobby_add_user">(*l, l->get_user_id(user), args);
    }
}

void game_manager::handle_message(MSG_TAG(lobby_make), game_user &user, const lobby_info &value) {
    if (user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    id_type lobby_id = ++m_lobby_count;
    auto &l = m_lobby_order.emplace_back(m_lobbies.try_emplace(lobby_id, value, lobby_id).first)->second;

    int user_id = l.add_user(user).user_id;

    l.state = lobby_state::waiting;
    broadcast_message<"lobby_update">(l);

    send_message<"lobby_entered">(user.client, user_id, l.lobby_id, l.name, l.options);
    send_message<"lobby_add_user">(user.client, user_id, user);
}

void game_manager::handle_message(MSG_TAG(lobby_edit), game_user &user, const lobby_info &args) {
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

    static_cast<lobby_info &>(lobby) = args;
    for (auto &[team, user_id, p] : lobby.users) {
        if (p != &user) {
            send_message<"lobby_edited">(p->client, args);
        }
    }
}

void game_manager::handle_join_lobby(game_user &user, lobby &lobby) {
    int new_user_id = lobby.add_user(user).user_id;

    broadcast_message<"lobby_update">(lobby);

    send_message<"lobby_entered">(user.client, new_user_id, lobby.lobby_id, lobby.name, lobby.options);
    for (auto &[team, user_id, p] : lobby.users) {
        if (p != &user) {
            send_message<"lobby_add_user">(p->client, new_user_id, user);
        }
        send_message<"lobby_add_user">(user.client, user_id, *p, true, p->get_disconnect_lifetime());
    }
    for (auto &bot : lobby.bots) {
        send_message<"lobby_add_user">(user.client, bot.user_id, bot, true);
    }
    for (const auto &message: lobby.chat_messages) {
        send_message<"lobby_chat">(user.client, message);
    }
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        player *target = lobby.m_game->find_player_by_userid(new_user_id);
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
        }
    }
}

void game_manager::handle_message(MSG_TAG(lobby_join), game_user &user, const lobby_id_args &value) {
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
    if (m_options.verbose) {
        fmt::print("{}: Connected\n", get_client_ip(client));
        fflush(stdout);
    }
    m_clients.emplace(client, client);
    broadcast_message<"client_count">(m_clients.size());
}

void game_manager::on_disconnect(client_handle client) {
    if (auto it = m_clients.find(client); it != m_clients.end()) {
        if (game_user *user = it->second.user) {
            user->client.reset();
            if (user->in_lobby) {
                lobby &lobby = *user->in_lobby;
                broadcast_message_lobby<"lobby_add_user">(lobby, lobby.get_user_id(*user), *user, true, user->get_disconnect_lifetime());
            }
        }
        m_clients.erase(it);
        broadcast_message<"client_count">(m_clients.size());
    }
}

void game_manager::handle_message(MSG_TAG(lobby_leave), game_user &user) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    kick_user_from_lobby(user);
}

void game_manager::handle_message(MSG_TAG(lobby_chat), game_user &user, const lobby_chat_client_args &value) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    if (!value.message.empty()) {
        auto &lobby = *user.in_lobby;
        int user_id = lobby.get_user_id(user);
        lobby.chat_messages.emplace_back(user_id, value.message, true);
        broadcast_message_lobby<"lobby_chat">(lobby, user_id, value.message);
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

void game_manager::handle_message(MSG_TAG(lobby_return), game_user &user) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = *user.in_lobby;

    if (&user != lobby.users.front().user) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state == lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_WAITING");
    }

    for (const auto &bot : lobby.bots) {
        broadcast_message_lobby<"lobby_remove_user">(lobby, bot.user_id);
    }
    lobby.bots.clear();
    lobby.m_game.reset();
    
    lobby.state = lobby_state::waiting;

    for (auto &[team, user_id, p] : lobby.users) {
        team = lobby_team::game_player;
    }

    broadcast_message<"lobby_update">(lobby);
    broadcast_message_lobby<"lobby_entered">(lobby, lobby.get_user_id(user), lobby.lobby_id, lobby.name, lobby.options);
}

void game_manager::handle_message(MSG_TAG(game_start), game_user &user) {
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

    if (m_options.verbose) {
        fmt::print("Started game {} with seed {}\n", lobby.name, lobby.m_game->rng_seed);
        fflush(stdout);
    }

    std::vector<int> user_ids;
    for (const auto &[team, user_id, user] : lobby.users) {
        if (team == lobby_team::game_player) {
            user_ids.push_back(user_id);
        }
    }

    auto names = bot_info.names
        | rv::sample(lobby.options.num_bots, lobby.m_game->bot_rng)
        | rn::to<std::vector<std::string_view>>;

    std::vector<const sdl::image_pixels *> propics = bot_info.propics
        | rv::transform([](const sdl::image_pixels &image) { return &image; })
        | rv::sample(lobby.options.num_bots, lobby.m_game->bot_rng)
        | rn::to_vector;

    for (int i=0; i < lobby.options.num_bots; ++i) {
        int bot_id = -1-i;
        auto &bot = lobby.bots.emplace_back(user_info{fmt::format("BOT {}", names[i % names.size()]), *propics[i % propics.size()] }, bot_id);
        user_ids.push_back(bot_id);

        broadcast_message_lobby<"lobby_add_user">(lobby, bot_id, bot, true);
    }

    lobby.m_game->add_players(user_ids);
    lobby.m_game->start_game(lobby.options);
    lobby.m_game->commit_updates();
}

void game_manager::handle_message(MSG_TAG(game_rejoin), game_user &user, int player_id) {
    auto &lobby = *user.in_lobby;

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    lobby_team &user_team = rn::find(lobby.users, &user, &lobby_user::user)->team;
    if (user_team != lobby_team::game_spectator) {
        throw lobby_error("ERROR_USER_NOT_SPECTATOR");
    }

    player *target = lobby.m_game->find_player(player_id);
    if (target->is_bot()) {
        throw lobby_error("ERROR_CANNOT_REJOIN_ON_BOT");
    }
    for (const auto &[team, user_id, user]: lobby.users) {
        if (user_id == target->user_id) {
            throw lobby_error("ERROR_PLAYER_NOT_REJOINABLE");
        }
    }

    user_team = lobby_team::game_player;
    target->user_id = lobby.get_user_id(user);

    lobby.m_game->add_update<game_update_type::player_add>(std::vector{to_player_user_pair(target)});
    
    for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
        send_message<"game_update">(user.client, msg);
    }
    for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
        send_message<"game_update">(user.client, msg);
    }
}

void game_manager::handle_message(MSG_TAG(game_action), game_user &user, const json::json &value) {
    if (!user.in_lobby) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = *user.in_lobby;

    if (lobby.state != lobby_state::playing || !lobby.m_game) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (lobby.m_game->is_waiting()) {
        throw lobby_error("ERROR_GAME_STATE_WAITING");
    }

    auto err = lobby.m_game->handle_game_action(lobby.get_user_id(user), value);
    if (!err.empty()) {
        throw lobby_error(err);
    }
}