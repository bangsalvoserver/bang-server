#include "manager.h"

#include <iostream>
#include <stdexcept>

#include <fstream>
#include <iomanip>
#include <ctime>

#include "utils/json_serial.h"
#include "net/git_version.h"

using namespace banggame;
using namespace enums::flag_operators;

void game_manager::on_receive_message(client_handle client, const client_message &msg) {
    try {
        enums::visit_indexed([&]<client_message_type E>(enums::enum_tag_t<E> tag, auto && ... args) {
            if constexpr (requires { handle_message(tag, client, FWD(args) ...); }) {
                handle_message(tag, client, FWD(args) ...);
            } else if (auto it = users.find(client); it != users.end()) {
                handle_message(tag, it, FWD(args) ...);
            } else {
                kick_client(client, "INVALID_MESSAGE");
            }
        }, msg);
    } catch (const lobby_error &e) {
        send_message<server_message_type::lobby_error>(client, e.what());
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
        if (l.state == lobby_state::finished) {
            send_lobby_update(it);
        }
        if (l.users.empty() && --l.lifetime == 0) {
            broadcast_message<server_message_type::lobby_removed>(it->first);
            it = m_lobbies.erase(it);
        } else {
            ++it;
        }
    }
}

void game_manager::handle_message(MSG_TAG(connect), client_handle client, const connect_args &args) {
    if (!net::validate_commit_hash(args.commit_hash)) {
        kick_client(client, "INVALID_CLIENT_COMMIT_HASH");
    } else if (auto [it, inserted] = users.try_emplace(client, ++m_user_counter, args.user_name, args.profile_image); inserted) {
        send_message<server_message_type::client_accepted>(client, it->second.user_id);
    }
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

void game_manager::handle_message(MSG_TAG(lobby_list), user_ptr user) {
    for (auto it = m_lobbies.begin(); it != m_lobbies.end(); ++it) {
        send_message<server_message_type::lobby_update>(user->first, make_lobby_data(it));
    }
}

void game_manager::handle_message(MSG_TAG(lobby_make), user_ptr user, const lobby_info &value) {
    if (user->second.in_lobby) {
        throw lobby_error("ERROR_PLAYER_IN_LOBBY");
    }

    auto lobby_it = m_lobbies.try_emplace(++m_lobby_counter).first;
    auto &new_lobby = lobby_it->second;

    new_lobby.users.push_back(user);
    user->second.in_lobby = lobby_it;

    static_cast<lobby_info &>(new_lobby) = value;
    new_lobby.owner = user;
    new_lobby.state = lobby_state::waiting;
    send_lobby_update(lobby_it);

    send_message<server_message_type::lobby_entered>(user->first, value);
    send_message<server_message_type::lobby_add_user>(user->first, user->second.user_id, user->second.name, user->second.profile_image);
    send_message<server_message_type::lobby_owner>(user->first, user->second.user_id);
}

void game_manager::handle_message(MSG_TAG(lobby_edit), user_ptr user, const lobby_info &args) {
    auto &lobby = user->second.get_lobby();

    if (lobby.owner != user) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    static_cast<lobby_info &>(lobby) = args;
    for (user_ptr p : lobby.users) {
        if (p != user) {
            send_message<server_message_type::lobby_edited>(p->first, args);
        }
    }
}

void game_manager::handle_message(MSG_TAG(lobby_join), user_ptr user, const lobby_id_args &value) {
    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        throw lobby_error("ERROR_INVALID_LOBBY");
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
        if (lobby.users.size() == 1) {
            lobby.owner = user;
        }
        send_message<server_message_type::lobby_owner>(user->first, lobby.owner->second.user_id);
        if (lobby.state != lobby_state::waiting) {
            send_message<server_message_type::game_started>(user->first);

            for (const auto &msg : lobby.game.get_spectator_updates()) {
                send_message<server_message_type::game_update>(user->first, msg);
            }
        }
    }
}

void game_manager::handle_message(MSG_TAG(lobby_rejoin), user_ptr user, const lobby_rejoin_args &value) {
    auto &lobby = user->second.get_lobby();

    player *target = lobby.game.find_player(value.player_id);
    if (!target || target->user_id != 0) return;

    target->user_id = user->second.user_id;

    broadcast_message_lobby<server_message_type::game_update>(lobby,
            enums::enum_tag<game_update_type::player_user>, target->id, target->user_id);
    
    for (const auto &msg : lobby.game.get_rejoin_updates(target)) {
        send_message<server_message_type::game_update>(user->first, msg);
    }
}

void game_manager::client_disconnected(client_handle client) {
    if (auto it = users.find(client); it != users.end()) {
        handle_message(MSG_TAG(lobby_leave){}, it);
        users.erase(it);
    }
}

bool game_manager::client_validated(client_handle client) const {
    return users.find(client) != users.end();
}

void game_manager::handle_message(MSG_TAG(lobby_leave), user_ptr user) {
    if (!user->second.in_lobby) return;

    auto lobby_it = *user->second.in_lobby;
    auto &lobby = lobby_it->second;
    user->second.in_lobby.reset();

    if (auto it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); it != lobby.game.m_players.end()) {
        it->user_id = 0;
        broadcast_message_lobby<server_message_type::game_update>(lobby, enums::enum_tag<game_update_type::player_user>, it->id, 0);
    }
    
    broadcast_message_lobby<server_message_type::lobby_remove_user>(lobby, user->second.user_id);
    lobby.users.erase(std::ranges::find(lobby.users, user));

    send_lobby_update(lobby_it);

    if (lobby.users.empty()) {
        if (lobby.state == lobby_state::playing) {
            lobby.lifetime = banggame::server_tickrate * lobby::lifetime_seconds;
        } else {
            broadcast_message<server_message_type::lobby_removed>(lobby_it->first);
            m_lobbies.erase(lobby_it);
        }
    } else if (user == lobby.owner) {
        lobby.owner = lobby.users.front();
        broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.owner->second.user_id);
    }
}

void game_manager::handle_message(MSG_TAG(lobby_chat), user_ptr user, const lobby_chat_client_args &value) {
    broadcast_message_lobby<server_message_type::lobby_chat>(user->second.get_lobby(), user->second.user_id, value.message);
}

void game_manager::handle_message(MSG_TAG(lobby_return), user_ptr user) {
    auto &lobby = user->second.get_lobby();

    if (user != lobby.owner) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::finished) {
        throw lobby_error("ERROR_LOBBY_NOT_FINISHED");
    }

    #ifdef DEBUG_PRINT_PUBLIC_LOGS
        if (!lobby.game.m_public_updates.empty()) {
            std::stringstream filename;
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            filename << "game_log_" << std::put_time(&tm, "%Y%m%d%H%M%S") << ".json";
            std::ofstream ofs(filename.str());
            Json::Value val = Json::arrayValue;
            for (const auto &obj : lobby.game.m_public_updates) {
                val.append(json::serialize(obj));
            }
            ofs << val;
        }
    #endif

    lobby.state = lobby_state::waiting;
    send_lobby_update(*(user->second.in_lobby));

    broadcast_message_lobby<server_message_type::lobby_entered>(lobby, lobby);
    for (user_ptr p : lobby.users) {
        broadcast_message_lobby<server_message_type::lobby_add_user>(lobby, p->second.user_id, p->second.name, p->second.profile_image);
    }
    broadcast_message_lobby<server_message_type::lobby_owner>(lobby, lobby.owner->second.user_id);
}

void game_manager::handle_message(MSG_TAG(game_start), user_ptr user) {
    auto &lobby = user->second.get_lobby();

    if (user != lobby.owner) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::waiting) {
        throw lobby_error("ERROR_LOBBY_NOT_WAITING");
    }

    if (lobby.users.size() <= 1) {
        throw lobby_error("ERROR_NOT_ENOUGH_PLAYERS");
    }

    lobby.state = lobby_state::playing;
    send_lobby_update(*(user->second.in_lobby));

    lobby.start_game(*this);
}

void game_manager::handle_message(MSG_TAG(game_action), user_ptr user, const game_action &value) {
    #ifdef DEBUG_PRINT_GAME_UPDATES
        std::cout << "/*** GAME ACTION *** ID = " << user->second.user_id << " ***/ " << json::serialize(value) << '\n';
    #endif

    auto &lobby = user->second.get_lobby();

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (auto it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); it != lobby.game.m_players.end()) {
        enums::visit_indexed([&]<game_action_type E>(enums::enum_tag_t<E> tag, auto && ... args) {
            it->handle_action(tag, FWD(args) ...);
        }, value);
    } else {
        throw lobby_error("ERROR_USER_NOT_CONTROLLING_PLAYER");
    }
}

void lobby::send_updates(game_manager &mgr) {
    while (state == lobby_state::playing && !game.m_updates.empty()) {
        auto &[target, update] = game.m_updates.front();
        if (update.is(game_update_type::game_over)) {
            state = lobby_state::finished;
        }
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

    game = {};
    
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