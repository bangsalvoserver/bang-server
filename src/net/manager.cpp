#include "manager.h"

#include <iostream>
#include <stdexcept>

#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>

#include "git_version.h"

using namespace banggame;

void game_manager::on_receive_message(client_handle client, const client_message &msg) {
    try {
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
        print_error(fmt::format("Error in game_manager: {}", e.what()));
    }
}

void game_manager::tick() {
    for (auto it = m_lobbies.begin(); it != m_lobbies.end();) {
        auto &l = it->second;
        if (l.state == lobby_state::playing) {
            l.game.tick();
        }
        l.send_updates(*this);
        if (l.game.check_flags(game_flags::game_over) && l.state != lobby_state::finished) {
            l.state = lobby_state::finished;
            send_lobby_update(it);
        }
        if (l.users.empty() && --l.lifetime == ticks{0}) {
            broadcast_message<server_message_type::lobby_removed>(it->first);
            it = m_lobbies.erase(it);
        } else {
            ++it;
        }
    }
}

std::string game_manager::handle_message(MSG_TAG(connect), client_handle client, const connect_args &args) {
    if (!net::validate_commit_hash(args.commit_hash)) {
        kick_client(client, "INVALID_CLIENT_COMMIT_HASH");
    } else if (auto [it, inserted] = users.try_emplace(client, ++m_user_counter, args.user_name, args.profile_image); inserted) {
        send_message<server_message_type::client_accepted>(client, it->second.user_id);
    }
    return {};
}

lobby_data game_manager::make_lobby_data(lobby_ptr it) {
    const lobby &l = it->second;
    lobby_data obj;
    obj.lobby_id = it->first;
    obj.name = l.name;
    obj.num_players = static_cast<int>(l.users.size());
    obj.state = l.state;
    return obj;
}

void game_manager::send_lobby_update(lobby_ptr it) {
    broadcast_message<server_message_type::lobby_update>(make_lobby_data(it));
}

std::string game_manager::handle_message(MSG_TAG(lobby_list), user_ptr user) {
    for (auto it = m_lobbies.begin(); it != m_lobbies.end(); ++it) {
        send_message<server_message_type::lobby_update>(user->first, make_lobby_data(it));
    }
    
    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_make), user_ptr user, const lobby_info &value) {
    if (user->second.in_lobby) {
        return "ERROR_PLAYER_IN_LOBBY";
    }

    auto lobby_it = m_lobbies.try_emplace(++m_lobby_counter).first;
    auto &new_lobby = lobby_it->second;

    new_lobby.users.push_back(user);
    user->second.in_lobby = lobby_it;

    static_cast<lobby_info &>(new_lobby) = value;
    new_lobby.state = lobby_state::waiting;
    send_lobby_update(lobby_it);

    send_message<server_message_type::lobby_entered>(user->first, value);
    send_message<server_message_type::lobby_add_user>(user->first, user->second.user_id, user->second.name, user->second.profile_image);
    send_message<server_message_type::lobby_owner>(user->first, user->second.user_id);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_edit), user_ptr user, const lobby_info &args) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = (*user->second.in_lobby)->second;

    if (lobby.users.front() != user) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    static_cast<lobby_info &>(lobby) = args;
    for (user_ptr p : lobby.users) {
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

    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        return "ERROR_INVALID_LOBBY";
    }

    auto &lobby = lobby_it->second;
    if (lobby.users.size() < lobby_max_players) {
        lobby.users.emplace_back(user);
        user->second.in_lobby = lobby_it;
        send_lobby_update(lobby_it);

        send_message<server_message_type::lobby_entered>(user->first, lobby);
        for (user_ptr p : lobby.users) {
            if (p != user) {
                send_message<server_message_type::lobby_add_user>(p->first, user->second.user_id, user->second.name, user->second.profile_image);
            }
            send_message<server_message_type::lobby_add_user>(user->first, p->second.user_id, p->second.name, p->second.profile_image);
        }
        send_message<server_message_type::lobby_owner>(user->first, lobby.users.front()->second.user_id);
        if (lobby.state != lobby_state::waiting) {
            send_message<server_message_type::game_started>(user->first);

            for (const auto &msg : lobby.game.get_spectator_updates()) {
                send_message<server_message_type::game_update>(user->first, msg);
            }
        }
    }

    return {};
}

std::string game_manager::handle_message(MSG_TAG(lobby_rejoin), user_ptr user, const lobby_rejoin_args &value) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = (*user->second.in_lobby)->second;

    if (ranges_contains(lobby.game.m_players, user->second.user_id, &player::user_id)) {
        return "ERROR_USER_CONTROLLING_PLAYER";
    }

    player *target = lobby.game.find_player(value.player_id);
    if (!target || target->user_id != 0) {
        return "ERROR_INVALID_REJOIN_TARGET";
    }

    target->user_id = user->second.user_id;

    broadcast_message_lobby<server_message_type::game_update>(lobby, lobby.game.make_update<game_update_type::player_user>(target, target->user_id));
    
    for (const auto &msg : lobby.game.get_rejoin_updates(target)) {
        send_message<server_message_type::game_update>(user->first, json::serialize(msg, lobby.game.context()));
    }

    return {};
}

void game_manager::kick_user_from_lobby(user_ptr user) {
    auto lobby_it = *user->second.in_lobby;
    auto &lobby = lobby_it->second;
    user->second.in_lobby.reset();

    if (auto it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); it != lobby.game.m_players.end()) {
        it->user_id = 0;
        broadcast_message_lobby<server_message_type::game_update>(lobby, lobby.game.make_update<game_update_type::player_user>(&*it, 0));
    }
    
    broadcast_message_lobby<server_message_type::lobby_remove_user>(lobby, user->second.user_id);

    auto it = std::ranges::find(lobby.users, user);
    bool is_owner = it == lobby.users.begin();
    lobby.users.erase(it);

    send_lobby_update(lobby_it);

    if (lobby.users.empty()) {
        if (lobby.state != lobby_state::playing) {
            broadcast_message<server_message_type::lobby_removed>(lobby_it->first);
            m_lobbies.erase(lobby_it);
        }
    } else if (is_owner) {
        broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.users.front()->second.user_id);
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
        auto &lobby = (*user->second.in_lobby)->second;
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

    if (bool(command.permissions() & command_permissions::lobby_owner)) {
        auto &lobby = (*user->second.in_lobby)->second;
        if (user != lobby.users.front()) {
            return "ERROR_PLAYER_NOT_LOBBY_OWNER";
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
    auto &lobby = (*user->second.in_lobby)->second;

    if (user != lobby.users.front()) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::finished) {
        return "ERROR_LOBBY_NOT_FINISHED";
    }

    lobby.game = {};
    lobby.state = lobby_state::waiting;
    send_lobby_update(*(user->second.in_lobby));

    broadcast_message_lobby<server_message_type::lobby_entered>(lobby, lobby);
    for (user_ptr p : lobby.users) {
        broadcast_message_lobby<server_message_type::lobby_add_user>(lobby, p->second.user_id, p->second.name, p->second.profile_image);
    }
    broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.users.front()->second.user_id);
    
    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_start), user_ptr user) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = (*user->second.in_lobby)->second;

    if (user != lobby.users.front()) {
        return "ERROR_PLAYER_NOT_LOBBY_OWNER";
    }

    if (lobby.state != lobby_state::waiting) {
        return "ERROR_LOBBY_NOT_WAITING";
    }

    if (lobby.users.size() <= 1) {
        return "ERROR_NOT_ENOUGH_PLAYERS";
    }

    lobby.state = lobby_state::playing;
    send_lobby_update(*(user->second.in_lobby));

    lobby.start_game(*this);

    return {};
}

std::string game_manager::handle_message(MSG_TAG(game_action), user_ptr user, const Json::Value &value) {
    if (!user->second.in_lobby) {
        return "ERROR_PLAYER_NOT_IN_LOBBY";
    }
    auto &lobby = (*user->second.in_lobby)->second;

    if (lobby.state != lobby_state::playing) {
        return "ERROR_LOBBY_NOT_PLAYING";
    }

    if (auto it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); it != lobby.game.m_players.end()) {
        if (auto error = enums::visit_indexed([&]<game_action_type E>(enums::enum_tag_t<E> tag, auto && ... args) {
            return it->handle_action(tag, FWD(args) ...);
        }, json::deserialize<banggame::game_action>(value, lobby.game.context()))) {
            lobby.game.add_update<game_update_type::game_error>(update_target::includes_private(&*it), std::move(error));
        }
    } else {
        return "ERROR_USER_NOT_CONTROLLING_PLAYER";
    }
    return {};
}

void lobby::send_updates(game_manager &mgr) {
    while (state == lobby_state::playing && !game.m_updates.empty()) {
        auto &[target, update] = game.m_updates.front();
        for (auto it : users) {
            if (target.matches(it->second.user_id)) {
                mgr.send_message<server_message_type::game_update>(it->first, update);
            }
        }
        game.m_updates.pop_front();
    }
}

void lobby::start_game(game_manager &mgr) {
    mgr.broadcast_message_lobby<server_message_type::game_started>(*this);
    
    std::vector<player *> ids;
    for (const auto &_ : users) {
        ids.push_back(&game.m_players.emplace(&game, static_cast<int>(game.m_players.first_available_id())));
    }
    std::ranges::shuffle(ids, game.rng);

    auto it = users.begin();
    for (player *p : ids) {
        p->user_id = (*it)->second.user_id;
        ++it;
    }

    game.start_game(options);
}