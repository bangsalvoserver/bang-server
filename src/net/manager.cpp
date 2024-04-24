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
        auto error = enums::visit_indexed([&]<client_message_type E>(enums::enum_tag_t<E> tag, auto && ... args) -> std::string {
            if constexpr (requires { handle_message(tag, client, args ...); }) {
                return handle_message(tag, client, FWD(args) ...);
            } else if (auto it = m_clients.find(client); it != m_clients.end()) {
                return handle_message(tag, *(it->second.user), FWD(args) ...);
            } else {
                kick_client(client, "CLIENT_NOT_VALIDATED");
                return {};
            }
        }, client_msg);
        if (!error.empty()) {
            send_message<server_message_type::lobby_error>(client, std::move(error));
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error in on_message(): {}\n", e.what());
    }
}

void game_manager::tick() {
    net::wsserver::tick();

    for (auto it = m_clients.begin(); it != m_clients.end(); ) {
        auto &[client, state] = *it;
        ++it;
        if (++state.ping_timer > ping_interval) {
            state.ping_timer = ticks{};
            if (++state.ping_count >= pings_until_disconnect) {
                if (state.user->in_lobby) {
                    kick_user_from_lobby(*(state.user));
                }
                kick_client(client, "INACTIVITY");
            } else {
                send_message<server_message_type::ping>(client);
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
                l.send_updates(*this);
                if (l.m_game->is_game_over()) {
                    l.state = lobby_state::finished;
                    send_lobby_update(l);
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
            broadcast_message<server_message_type::lobby_removed>(l.lobby_id);
            m_lobbies.erase(*it);
            it = m_lobby_order.erase(it);
            continue;
        }
        ++it;
    }
}

static std::random_device global_rng;
static constexpr int max_session_id_count = 10;

std::string game_manager::handle_message(MSG_TAG(connect), client_handle client, const connect_args &args) {
    if (!net::validate_commit_hash(args.commit_hash)) {
        kick_client(client, "INVALID_CLIENT_COMMIT_HASH");
        return {};
    }
    id_type session_id = args.session_id;
    if (session_id == 0) {
        int i = 0;
        while (i < max_session_id_count) {
            session_id = global_rng();
            if (session_id != 0 && m_users.find(session_id) == m_users.end()) {
                break;
            }
        }
        if (i >= max_session_id_count) {
            kick_client(client, "CANNOT_VALIDATE_CLIENT");
            throw std::runtime_error("Surpassed max_count in generate_random_id");
        }
    }
    game_user *user = nullptr;
    if (auto it = m_users.find(session_id); it != m_users.end()) {
        user = &it->second;
        client_handle found = user->client;
        if (found.lock().get() == client.lock().get()) {
            return "USER_ALREADY_CONNECTED";
        }
        kick_client(found, "RECONNECT_WITH_SAME_SESSION_ID");
        static_cast<user_info &>(*user) = args.user;
    } else {
        user = &m_users.emplace(std::piecewise_construct,
            std::make_tuple(session_id),
            std::make_tuple(args.user, session_id)
        ).first->second;
    }
    user->client = client;
    m_clients.emplace(client, user);
    
    send_message<server_message_type::client_accepted>(client, session_id);
    broadcast_message<server_message_type::client_count>(m_clients.size());
    for (const auto it : m_lobby_order) {
        send_message<server_message_type::lobby_update>(client, it->second.make_lobby_data());
    }

    if (user->in_lobby) {
        handle_join_lobby(*user, *user->in_lobby);
    }

    return {};
}

std::string game_manager::handle_message(MSG_TAG(pong), client_handle client) {
    if (auto it = m_clients.find(client); it != m_clients.end()) {
        it->second.ping_count = 0;   
    }
    return {};
}

std::string game_manager::handle_message(MSG_TAG(user_edit), game_user &user, const user_info &args) {
    static_cast<user_info &>(user) = args;

    if (auto *l = user.in_lobby) {
        broadcast_message_lobby<server_message_type::lobby_add_user>(*l, l->get_user_id(user), args);
    }

    return {};
}

lobby_data lobby::make_lobby_data() const {
    return {
        .lobby_id = lobby_id,
        .name = name,
        .num_players = int(users.size()),
        .state = state
    };
}

void game_manager::send_lobby_update(const lobby &l) {
    broadcast_message<server_message_type::lobby_update>(l.make_lobby_data());
}

std::string game_manager::handle_message(MSG_TAG(lobby_make), game_user &user, const lobby_info &value) {
    if (user.in_lobby) {
        return "ERROR_PLAYER_IN_LOBBY";
    }

    id_type lobby_id = ++m_lobby_count;
    auto &l = m_lobby_order.emplace_back(m_lobbies.try_emplace(lobby_id, value, lobby_id).first)->second;

    int user_id = ++l.user_id_count;

    l.users.emplace_back(lobby_team::game_player, user_id, &user);
    user.in_lobby = &l;

    l.state = lobby_state::waiting;
    send_lobby_update(l);

    send_message<server_message_type::lobby_entered>(user.client, user_id, l.lobby_id, l.name, l.options);
    send_message<server_message_type::lobby_add_user>(user.client, user_id, user);
    send_message<server_message_type::lobby_owner>(user.client, user_id);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_edit), game_user &user, const lobby_info &args) {
    if (!user.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user.in_lobby;

    if (lobby.users.front().user != &user) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    static_cast<lobby_info &>(lobby) = args;
    for (auto &[team, user_id, p] : lobby.users) {
        if (p != &user) {
            send_message<server_message_type::lobby_edited>(p->client, args);
        }
    }

    return {};
}

lobby_user &lobby::add_user(game_user &user) {
    if (auto it = rn::find(users, &user, &lobby_user::user); it != users.end()) {
        return *it;
    } else {
        return users.emplace_back(lobby_team::game_player, ++user_id_count, &user);
    }
}

void game_manager::handle_join_lobby(game_user &user, lobby &lobby) {
    auto &pair = lobby.add_user(user);

    user.in_lobby = &lobby;
    send_lobby_update(lobby);

    send_message<server_message_type::lobby_entered>(user.client, pair.user_id, lobby.lobby_id, lobby.name, lobby.options);
    for (auto &[team, user_id, p] : lobby.users) {
        if (p != &user) {
            send_message<server_message_type::lobby_add_user>(p->client, pair.user_id, user);
        }
        send_message<server_message_type::lobby_add_user>(user.client, user_id, *p, true);
    }
    for (auto &bot : lobby.bots) {
        send_message<server_message_type::lobby_add_user>(user.client, bot.user_id, bot, true);
    }
    send_message<server_message_type::lobby_owner>(user.client, lobby.users.front().user_id);
    for (const auto &message: lobby.chat_messages) {
        send_message<server_message_type::lobby_chat>(user.client, message);
    }
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        pair.team = lobby_team::game_spectator;
        send_message<server_message_type::game_started>(user.client);

        for (const auto &msg : lobby.m_game->get_spectator_join_updates()) {
            send_message<server_message_type::game_update>(user.client, msg);
        }
        player *target = lobby.m_game->find_player_by_userid(pair.user_id);
        if (target) {
            for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
                send_message<server_message_type::game_update>(user.client, msg);
            }
        }
        for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
            send_message<server_message_type::game_update>(user.client, msg);
        }
    }
}

std::string game_manager::handle_message(MSG_TAG(lobby_join), game_user &user, const lobby_id_args &value) {
    if (user.in_lobby) {
        return "ERROR_PLAYER_IN_LOBBY";
    }

    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        return "ERROR_INVALID_LOBBY";
    }

    handle_join_lobby(user, lobby_it->second);

    return {};
}

void game_manager::kick_user_from_lobby(game_user &user) {
    auto &lobby = *std::exchange(user.in_lobby, nullptr);

    auto it = rn::find(lobby.users, &user, &lobby_user::user);
    bool is_owner = it == lobby.users.begin();
    int user_id = it->user_id;
    lobby.users.erase(it);

    send_lobby_update(lobby);
    broadcast_message_lobby<server_message_type::lobby_remove_user>(lobby, user_id);
    send_message<server_message_type::lobby_kick>(user.client);

    if (!lobby.users.empty() && is_owner) {
        broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.users.front().user_id);
    }
}

void game_manager::on_connect(client_handle client) {
    if (m_options.verbose) {
        fmt::print("{}: Connected\n", get_client_ip(client));
        fflush(stdout);
    }
}

void game_manager::on_disconnect(client_handle client) {
    if (auto it = m_clients.find(client); it != m_clients.end()) {
        it->second.user->client.reset();
        m_clients.erase(it);
        broadcast_message<server_message_type::client_count>(m_clients.size());
    }
}

std::string game_manager::handle_message(MSG_TAG(lobby_leave), game_user &user) {
    if (!user.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }

    kick_user_from_lobby(user);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_chat), game_user &user, const lobby_chat_client_args &value) {
    if (!user.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    if (!value.message.empty()) {
        auto &lobby = *user.in_lobby;
        int user_id = lobby.get_user_id(user);
        lobby.chat_messages.emplace_back(user_id, value.message, true);
        broadcast_message_lobby<server_message_type::lobby_chat>(lobby, user_id, value.message);
        if (value.message[0] == chat_command::start_char) {
            return handle_chat_command(user, value.message.substr(1));
        }
    }
    return {};
}

std::string game_manager::handle_chat_command(game_user &user, const std::string &message) {
    size_t space_pos = message.find_first_of(" \t");
    auto cmd_name = std::string_view(message).substr(0, space_pos);
    auto cmd_it = rn::find(chat_command::commands, cmd_name, &string_command_map::value_type::first);
    if (cmd_it == chat_command::commands.end()) {
        return "INVALID_COMMAND_NAME";
    }

    auto &command = cmd_it->second;
    auto &lobby = *user.in_lobby;

    if (bool(command.permissions() & command_permissions::lobby_owner) && &user != lobby.users.front().user) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (bool(command.permissions() & command_permissions::lobby_waiting) && lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    if (bool(command.permissions() & command_permissions::lobby_playing) && lobby.state != lobby_state::playing) {
        return "ERROR_LOBBY_NOT_PLAYING";
    }

    if (bool(command.permissions() & command_permissions::lobby_finished) && lobby.state != lobby_state::finished) {
        return "ERROR_LOBBY_NOT_FINISHED";
    }

    if (bool(command.permissions() & command_permissions::game_cheat)) {
        if (lobby.state != lobby_state::playing) {
            return "ERROR_LOBBY_NOT_PLAYING";
        } else if (!m_options.enable_cheats) {
            return "ERROR_GAME_CHEATS_NOT_ENABLED";
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

    return command(this, user, args);
}

std::string game_manager::handle_message(MSG_TAG(lobby_return), game_user &user) {
    if (!user.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user.in_lobby;

    if (&user != lobby.users.front().user) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state == lobby_state::waiting) {
        return "ERROR_LOBBY_WAITING";
    }

    for (const auto &bot : lobby.bots) {
        broadcast_message_lobby<server_message_type::lobby_remove_user>(lobby, bot.user_id);
    }
    lobby.bots.clear();
    lobby.m_game.reset();
    
    lobby.state = lobby_state::waiting;
    send_lobby_update(*user.in_lobby);

    for (auto &[team, user_id, p] : lobby.users) {
        team = lobby_team::game_player;
    }

    broadcast_message_lobby<server_message_type::lobby_entered>(lobby, lobby.get_user_id(user), lobby.lobby_id, lobby.name, lobby.options);
    
    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_start), game_user &user) {
    if (!user.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user.in_lobby;

    if (&user != lobby.users.front().user) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    size_t num_players = rn::count(lobby.users, lobby_team::game_player, &lobby_user::team) + lobby.options.num_bots;

    if (num_players < 3) {
        return "ERROR_NOT_ENOUGH_PLAYERS";
    } else if (num_players > lobby_max_players) {
        return "ERROR_TOO_MANY_PLAYERS";
    }

    lobby.state = lobby_state::playing;
    send_lobby_update(*user.in_lobby);

    lobby.start_game(*this);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_rejoin), game_user &user, int player_id) {
    auto &lobby = *user.in_lobby;

    if (lobby.state != lobby_state::playing) {
        return "ERROR_LOBBY_NOT_PLAYING";
    }

    lobby_team &user_team = rn::find(lobby.users, &user, &lobby_user::user)->team;
    if (user_team != lobby_team::game_spectator) {
        return "ERROR_USER_NOT_SPECTATOR";
    }

    player *target = lobby.m_game->context().find_player(player_id);
    for (const auto &[team, user_id, user]: lobby.users) {
        if (user_id == target->user_id) {
            return "ERROR_PLAYER_NOT_REJOINABLE";
        }
    }

    user_team = lobby_team::game_player;
    target->user_id = lobby.get_user_id(user);

    lobby.m_game->add_update<game_update_type::player_add>(std::vector{player_user_pair{ target }});
    
    for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
        send_message<server_message_type::game_update>(user.client, msg);
    }
    for (const auto &msg : lobby.m_game->get_game_log_updates(target)) {
        send_message<server_message_type::game_update>(user.client, msg);
    }

    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_action), game_user &user, const json::json &value) {
    if (!user.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user.in_lobby;

    if (lobby.state != lobby_state::playing || !lobby.m_game) {
        return "ERROR_LOBBY_NOT_PLAYING";
    }

    if (lobby.m_game->is_waiting()) {
        return "ERROR_GAME_STATE_WAITING";
    }

    return lobby.m_game->handle_game_action(lobby.get_user_id(user), value);
}

void lobby::send_updates(game_manager &mgr) {
    while (state == lobby_state::playing && m_game && m_game->pending_updates()) {
        auto [target, update, update_time] = m_game->get_next_update();
        for (auto &[team, user_id, u] : users) {
            if (target.matches(user_id)) {
                mgr.send_message<server_message_type::game_update>(u->client, update);
            }
        }
    }
}

void lobby::start_game(game_manager &mgr) {
    mgr.broadcast_message_lobby<server_message_type::game_started>(*this);

    m_game = std::make_unique<banggame::game>(options.game_seed);

    if (mgr.m_options.verbose) {
        fmt::print("Started game {} with seed {}\n", name, m_game->rng_seed);
        fflush(stdout);
    }

    std::vector<int> user_ids;
    for (const auto &[team, user_id, user] : users) {
        if (team == lobby_team::game_player) {
            user_ids.push_back(user_id);
        }
    }

    auto names = bot_info.names
        | rv::sample(options.num_bots, m_game->rng)
        | rn::to<std::vector<std::string_view>>;

    std::vector<const sdl::image_pixels *> propics = bot_info.propics
        | rv::transform([](const sdl::image_pixels &image) { return &image; })
        | rv::sample(options.num_bots, m_game->rng)
        | rn::to_vector;

    for (int i=0; i<options.num_bots; ++i) {
        int bot_id = -1-i;
        auto &bot = bots.emplace_back(user_info{fmt::format("BOT {}", names[i % names.size()]), *propics[i % propics.size()] }, bot_id);
        user_ids.push_back(bot_id);

        mgr.broadcast_message_lobby<server_message_type::lobby_add_user>(*this, bot_id, bot, true);
    }

    m_game->add_players(user_ids);
    m_game->start_game(options);
    m_game->commit_updates();
}