#include "manager.h"

#include <iostream>
#include <stdexcept>

#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>

#include "git_version.h"
#include "bot_names.h"

using namespace banggame;

game_manager::game_manager() {
    std::random_device rd;
    m_rng.seed(rd());
}

void game_manager::on_receive_message(client_handle client, const client_message &msg) {
    try {
        if (m_options.verbose) {
            std::cout << client.lock().get() << ": Received " << json::serialize(msg) << std::endl;
        }
        auto error = enums::visit_indexed([&]<client_message_type E>(enums::enum_tag_t<E> tag, auto && ... args) {
            if constexpr (requires { handle_message(tag, client, args ...); }) {
                return handle_message(tag, client, FWD(args) ...);
            } else if (auto it = users.find(client); it != users.end()) {
                return handle_message(tag, it, FWD(args) ...);
            } else {
                kick_client(client, "INVALID_MESSAGE");
                return std::string();
            }
        }, msg);
        if (!error.empty()) {
            send_message<server_message_type::lobby_error>(client, std::move(error));
        }
    } catch (const std::exception &e) {
        std::cerr << fmt::format("Error in game_manager: {}", e.what()) << std::endl;
    }
}

void game_manager::tick() {
    for (auto it = m_lobbies.begin(); it != m_lobbies.end();) {
        if (it->state == lobby_state::playing && it->m_game) {
            it->m_game->tick();
            it->send_updates(*this);
            if (it->m_game->is_game_over()) {
                it->state = lobby_state::finished;
                send_lobby_update(*it);
            }
        }
        if (it->users.empty() && --it->lifetime == ticks{0}) {
            broadcast_message<server_message_type::lobby_removed>(it->id);
            it = m_lobbies.erase(it);
        } else {
            ++it;
        }
    }
}

std::string game_manager::handle_message(MSG_TAG(connect), client_handle client, const connect_args &args) {
    if (!net::validate_commit_hash(args.commit_hash)) {
        kick_client(client, "INVALID_CLIENT_COMMIT_HASH");
    } else if (auto [it, inserted] = users.try_emplace(client, generate_user_id(args.user_id), args.user); inserted) {
        send_message<server_message_type::client_accepted>(client, it->second.user_id);
    } else {
        return "USER_ALREADY_CONNECTED";
    }
    return {};
}

std::string game_manager::handle_message(MSG_TAG(user_edit), user_ptr user, const user_info &args) {
    user->second.user = args;

    if (user->second.in_lobby) {
        broadcast_message_lobby<server_message_type::lobby_add_user>(*user->second.in_lobby, user->second.user_id, args);
    }

    return {};
}

lobby_data lobby::make_lobby_data() const {
    return {
        .lobby_id = id,
        .name = name,
        .num_players = int(users.size()),
        .state = state
    };
}

void game_manager::send_lobby_update(const lobby &l) {
    broadcast_message<server_message_type::lobby_update>(l.make_lobby_data());
}

std::string game_manager::handle_message(MSG_TAG(lobby_list), user_ptr user) {
    for (const lobby &l : m_lobbies) {
        send_message<server_message_type::lobby_update>(user->first, l.make_lobby_data());
    }
    
    return {};
}

int game_manager::generate_lobby_id() {
    std::uniform_int_distribution dist{1, std::numeric_limits<int>::max()};
    while (true) {
        int lobby_id = dist(m_rng);
        if (!ranges::contains(m_lobbies, lobby_id, &lobby::id)) {
            return lobby_id;
        }
    }
}

int game_manager::generate_user_id(int user_id) {
    std::uniform_int_distribution dist{1, std::numeric_limits<int>::max()};
    while (user_id <= 0 || ranges::contains(users | ranges::views::values, user_id, &game_user::user_id)) {
        user_id = dist(m_rng);
    }
    return user_id;
}

std::string game_manager::handle_message(MSG_TAG(lobby_make), user_ptr user, const lobby_info &value) {
    if (user->second.in_lobby) {
        return "ERROR_PLAYER_IN_LOBBY";
    }

    auto &l = m_lobbies.emplace_back(generate_lobby_id(), value);

    l.users.emplace_back(lobby_team::game_player, user);
    user->second.in_lobby = &l;

    l.state = lobby_state::waiting;
    send_lobby_update(l);

    send_message<server_message_type::lobby_entered>(user->first, l.id, l.name, l.options);
    send_message<server_message_type::lobby_add_user>(user->first, user->second);
    send_message<server_message_type::lobby_owner>(user->first, user->second.user_id);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_edit), user_ptr user, const lobby_info &args) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user->second.in_lobby;

    if (lobby.users.front().second != user) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    static_cast<lobby_info &>(lobby) = args;
    for (auto &[team, p] : lobby.users) {
        if (p != user) {
            send_message<server_message_type::lobby_edited>(p->first, args);
        }
    }

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_join), user_ptr user, const lobby_id_args &value) {
    if (user->second.in_lobby) {
        return "ERROR_PLAYER_IN_LOBBY";
    }

    auto lobby_it = std::ranges::find(m_lobbies, value.lobby_id, &lobby::id);
    if (lobby_it == m_lobbies.end()) {
        return "ERROR_INVALID_LOBBY";
    }

    auto &lobby = *lobby_it;
    auto &pair = lobby.users.emplace_back(lobby_team::game_player, user);

    user->second.in_lobby = &lobby;
    send_lobby_update(lobby);

    send_message<server_message_type::lobby_entered>(user->first, lobby.id, lobby.name, lobby.options);
    for (auto &[team, p] : lobby.users) {
        if (p != user) {
            send_message<server_message_type::lobby_add_user>(p->first, user->second);
        }
        send_message<server_message_type::lobby_add_user>(user->first, p->second);
    }
    for (auto &bot : lobby.bots) {
        send_message<server_message_type::lobby_add_user>(user->first, bot);
    }
    send_message<server_message_type::lobby_owner>(user->first, lobby.users.front().second->second.user_id);
    
    if (lobby.state != lobby_state::waiting && lobby.m_game) {
        pair.first = lobby_team::game_spectator;
        send_message<server_message_type::game_started>(user->first);

        for (const auto &msg : lobby.m_game->get_spectator_updates()) {
            send_message<server_message_type::game_update>(user->first, msg);
        }

        if (player *target = lobby.m_game->find_player_by_userid(user->second.user_id)) {
            for (const auto &msg : lobby.m_game->get_rejoin_updates(target)) {
                send_message<server_message_type::game_update>(user->first, msg);
            }
        }
    }

    return {};
}

void game_manager::kick_user_from_lobby(user_ptr user) {
    auto &lobby = *std::exchange(user->second.in_lobby, nullptr);
    
    auto it = std::ranges::find(lobby.users, user, &team_user_pair::second);
    bool is_owner = it == lobby.users.begin();
    lobby.users.erase(it);

    send_lobby_update(lobby);
    broadcast_message_lobby<server_message_type::lobby_remove_user>(lobby, user->second.user_id);
    send_message<server_message_type::lobby_remove_user>(user->first, user->second.user_id);

    if (!lobby.users.empty() && is_owner) {
        broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.users.front().second->second.user_id);
    }
}

void game_manager::client_disconnected(client_handle client) {
    if (auto it = users.find(client); it != users.end()) {
        if (it->second.in_lobby) {
            kick_user_from_lobby(it);
        }
        users.erase(it);
    }
}

bool game_manager::client_validated(client_handle client) const {
    return users.find(client) != users.end();
}

std::string game_manager::handle_message(MSG_TAG(lobby_leave), user_ptr user) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }

    kick_user_from_lobby(user);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_chat), user_ptr user, const lobby_chat_client_args &value) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    if (!value.message.empty()) {
        auto &lobby = *user->second.in_lobby;
        broadcast_message_lobby<server_message_type::lobby_chat>(lobby, user->second.user_id, value.message);
        if (value.message[0] == chat_command::start_char) {
            return handle_chat_command(user, value.message.substr(1));
        }
    }
    return {};
}

std::string game_manager::handle_chat_command(user_ptr user, const std::string &message) {
    size_t space_pos = message.find_first_of(" \t");
    auto cmd_name = std::string_view(message).substr(0, space_pos);
    auto cmd_it = chat_command::commands.find(cmd_name);
    if (cmd_it == chat_command::commands.end()) {
        return "INVALID_COMMAND_NAME";
    }

    auto &command = cmd_it->second;
    auto &lobby = *user->second.in_lobby;

    if (bool(command.permissions() & command_permissions::lobby_owner) && user != lobby.users.front().second) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (bool(command.permissions() & command_permissions::lobby_waiting) && lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
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

std::string game_manager::handle_message(MSG_TAG(lobby_return), user_ptr user) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user->second.in_lobby;

    if (user != lobby.users.front().second) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::finished) {
        return "ERROR_LOBBY_NOT_FINISHED";
    }

    lobby.bots.clear();
    lobby.m_game.reset();
    
    lobby.state = lobby_state::waiting;
    send_lobby_update(*user->second.in_lobby);

    for (auto &[team, p] : lobby.users) {
        team = lobby_team::game_player;
    }

    broadcast_message_lobby<server_message_type::lobby_entered>(lobby, lobby.id, lobby.name, lobby.options);
    for (auto &[team, p] : lobby.users) {
        broadcast_message_lobby<server_message_type::lobby_add_user>(lobby, p->second);
    }
    broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.users.front().second->second.user_id);
    
    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_start), user_ptr user) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user->second.in_lobby;

    if (user != lobby.users.front().second) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    size_t num_players = std::ranges::count(lobby.users, lobby_team::game_player, &team_user_pair::first) + lobby.options.num_bots;

    if (num_players <= 1) {
        return "ERROR_NOT_ENOUGH_PLAYERS";
    } else if (num_players > lobby_max_players) {
        return "ERROR_TOO_MANY_PLAYERS";
    }

    lobby.state = lobby_state::playing;
    send_lobby_update(*user->second.in_lobby);

    lobby.start_game(*this);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_action), user_ptr user, const json::json &value) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = *user->second.in_lobby;

    if (lobby.state != lobby_state::playing || !lobby.m_game) {
        return "ERROR_LOBBY_NOT_PLAYING";
    }

    if (lobby.m_game->pending_updates()) {
        return "ERROR_GAME_PENDING_UPDATES";
    }

    if (player *origin = lobby.m_game->find_player_by_userid(user->second.user_id)) {
        origin->handle_game_action(json::deserialize<banggame::game_action>(value, lobby.m_game->context()));
        return {};
    } else {
        return "ERROR_USER_NOT_CONTROLLING_PLAYER";
    }
}

void lobby::send_updates(game_manager &mgr) {
    while (state == lobby_state::playing && m_game && !m_game->m_updates.empty()) {
        auto &[target, update, update_time] = m_game->m_updates.front();
        for (auto &[team, it] : users) {
            if (target.matches(it->second.user_id)) {
                mgr.send_message<server_message_type::game_update>(it->first, update);
            }
        }
        m_game->m_updates.pop_front();
    }
}

void lobby::start_game(game_manager &mgr) {
    mgr.broadcast_message_lobby<server_message_type::game_started>(*this);

    m_game = std::make_unique<banggame::game>();

    std::vector<int> user_ids;
    for (const team_user_pair &pair : users) {
        if (pair.first == lobby_team::game_player) {
            user_ids.push_back(pair.second->second.user_id);
        }
    }

    std::vector<std::string_view> names = bot_names
        | ranges::views::sample(options.num_bots, m_game->rng)
        | ranges::to<std::vector>;

    std::vector<const sdl::image_pixels *> propics = bot_profile_pictures
        | ranges::views::transform([](const sdl::image_pixels &image) { return &image; })
        | ranges::views::sample(options.num_bots, m_game->rng)
        | ranges::to<std::vector>;

    for (int i=0; i<options.num_bots; ++i) {
        auto &bot = bots.emplace_back(-1 - i, user_info{fmt::format("BOT {}", names[i % names.size()]), *propics[i % propics.size()] });
        user_ids.push_back(bot.user_id);

        mgr.broadcast_message_lobby<server_message_type::lobby_add_user>(*this, bot);
    }

    m_game->add_players(user_ids);
    m_game->start_game(options);
    m_game->update();
}