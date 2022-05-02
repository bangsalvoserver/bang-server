#include "manager.h"

#include <iostream>
#include <stdexcept>

#include "utils/json_serial.h"

using namespace banggame;
using namespace enums::flag_operators;

struct lobby_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

void game_manager::handle_message(client_handle client, const client_message &msg) {
    try {
        enums::visit_indexed([&](enums::enum_tag_for<client_message_type> auto tag, auto && ... args) {
            if constexpr (requires { handle_message(tag, client, std::forward<decltype(args)>(args) ...); }) {
                handle_message(tag, client, std::forward<decltype(args)>(args) ...);
            } else if (auto it = users.find(client); it != users.end()) {
                handle_message(tag, it, std::forward<decltype(args)>(args) ...);
            }
        }, msg);
    } catch (const lobby_error &e) {
        send_message<server_message_type::lobby_error>(client, e.what());
    } catch (const game_error &e) {
        send_message<server_message_type::game_update>(client, enums::enum_tag_t<game_update_type::game_error>(), e);
    }
}

void game_manager::start(std::stop_token token) {
    using frames = std::chrono::duration<int64_t, std::ratio<1, banggame::fps>>;
    auto next_frame = std::chrono::steady_clock::now() + frames{0};

    while (!token.stop_requested()) {
        next_frame += frames{1};

        while (auto msg = m_in_queue.pop_front()) {
            try {
                handle_message(msg->client, msg->value);
            } catch (const std::exception &error) {
                // print_error(fmt::format("Error: {}", error.what()));
            }
        }

        for (auto it = m_lobbies.begin(); it != m_lobbies.end(); ++it) {
            auto &l = it->second;
            if (l.state == lobby_state::playing) {
                l.game.tick();
            }
            l.send_updates(*this);
            if (l.state == lobby_state::finished) {
                send_lobby_update(it);
            }
        }

        std::this_thread::sleep_until(next_frame);
    }
}

void game_manager::HANDLE_MESSAGE(connect, client_handle client, const connect_args &args) {
    if (auto [it, inserted] = users.try_emplace(client, ++game_user::user_id_counter, args.user_name, args.profile_image); inserted) {
        send_message<server_message_type::client_accepted>(client, it->second.user_id);
    }
}

lobby_data game_manager::make_lobby_data(lobby_ptr it) {
    const lobby &l = it->second;
    lobby_data obj;
    obj.lobby_id = it->first;
    obj.name = l.name;
    obj.num_players = l.users.size();
    obj.state = l.state;
    return obj;
}

void game_manager::send_lobby_update(lobby_ptr it) {
    auto msg = make_message<server_message_type::lobby_update>(make_lobby_data(it));
    for (client_handle client : users | std::views::keys) {
        m_send_message(client, msg);
    }
}

void game_manager::HANDLE_MESSAGE(lobby_list, user_ptr user) {
    for (auto it = m_lobbies.begin(); it != m_lobbies.end(); ++it) {
        send_message<server_message_type::lobby_update>(user->first, make_lobby_data(it));
    }
}

void game_manager::HANDLE_MESSAGE(lobby_make, user_ptr user, const lobby_info &value) {
    if (user->second.in_lobby != lobby_ptr{}) {
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

    send_message<server_message_type::lobby_entered>(user->first, value, user->second.user_id);
    send_message<server_message_type::lobby_add_user>(user->first, user->second.user_id, user->second.name, user->second.profile_image);
}

void game_manager::HANDLE_MESSAGE(lobby_edit, user_ptr user, const lobby_info &args) {
    if (user->second.in_lobby == lobby_ptr{}) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    auto &lobby = user->second.in_lobby->second;

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

void game_manager::HANDLE_MESSAGE(lobby_join, user_ptr user, const lobby_join_args &value) {
    auto lobby_it = m_lobbies.find(value.lobby_id);
    if (lobby_it == m_lobbies.end()) {
        throw lobby_error("ERROR_INVALID_LOBBY");
    }

    auto &lobby = lobby_it->second;
    if (lobby.users.size() < lobby_max_players) {
        lobby.users.emplace_back(user);
        user->second.in_lobby = lobby_it;
        send_lobby_update(lobby_it);

        send_message<server_message_type::lobby_entered>(user->first, lobby, lobby.owner->second.user_id);
        for (user_ptr p : lobby.users) {
            if (p != user) {
                send_message<server_message_type::lobby_add_user>(p->first, user->second.user_id, user->second.name, user->second.profile_image);
            }
            send_message<server_message_type::lobby_add_user>(user->first, p->second.user_id, p->second.name, p->second.profile_image);
        }
        if (lobby.state != lobby_state::waiting) {
            send_message<server_message_type::game_started>(user->first, lobby.game.m_options);

            player *controlling = lobby.game.find_disconnected_player();
            if (controlling) {
                controlling->user_id = user->second.user_id;
                broadcast_message<server_message_type::game_update>(lobby,
                    enums::enum_tag<game_update_type::player_add>, controlling->id, controlling->user_id);
            }

            for (const player &p : lobby.game.m_players) {
                if (&p != controlling && (p.alive() || lobby.game.has_expansion(card_expansion_type::ghostcards))) {
                    send_message<server_message_type::game_update>(user->first, enums::enum_tag<game_update_type::player_add>,
                        p.id, p.user_id);
                }
            }

            for (const auto &msg : lobby.game.get_game_state_updates(controlling)) {
                send_message<server_message_type::game_update>(user->first, msg);
            }
        }
    }
}

void game_manager::client_disconnected(client_handle client) {
    if (auto it = users.find(client); it != users.end()) {
        handle_message(MESSAGE_TAG(lobby_leave){}, it);
        users.erase(it);
    }
}

bool game_manager::client_validated(client_handle client) const {
    return users.find(client) != users.end();
}

void game_manager::HANDLE_MESSAGE(lobby_leave, user_ptr user) {
    if (user->second.in_lobby == lobby_ptr{}) return;

    auto lobby_it = std::exchange(user->second.in_lobby, lobby_ptr{});
    auto &lobby = lobby_it->second;

    if (auto it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); it != lobby.game.m_players.end()) {
        it->user_id = 0;
    }
    
    broadcast_message<server_message_type::lobby_remove_user>(lobby, user->second.user_id);
    lobby.users.erase(std::ranges::find(lobby.users, user));
    
    if (lobby.state == lobby_state::waiting && user == lobby.owner) {
        for (user_ptr u : lobby.users) {
            broadcast_message<server_message_type::lobby_remove_user>(lobby, u->second.user_id);
            u->second.in_lobby = lobby_ptr{};
        }
        lobby.users.clear();
    }

    send_lobby_update(lobby_it);

    if (lobby.users.empty()) {
        m_lobbies.erase(lobby_it);
    }
}

void game_manager::HANDLE_MESSAGE(lobby_chat, user_ptr user, const lobby_chat_client_args &value) {
    if (user->second.in_lobby == lobby_ptr{}) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    broadcast_message<server_message_type::lobby_chat>(user->second.in_lobby->second, user->second.user_id, value.message);
}

void game_manager::HANDLE_MESSAGE(lobby_return, user_ptr user) {
    if (user->second.in_lobby == lobby_ptr{}) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    auto &lobby = user->second.in_lobby->second;

    if (user != lobby.owner) {
        throw lobby_error("ERROR_PLAYER_NOT_LOBBY_OWNER");
    }

    if (lobby.state != lobby_state::finished) {
        throw lobby_error("ERROR_LOBBY_NOT_FINISHED");
    }

    lobby.state = lobby_state::waiting;
    send_lobby_update(user->second.in_lobby);

    broadcast_message<server_message_type::lobby_entered>(lobby, lobby, user->second.user_id);
    for (user_ptr p : lobby.users) {
        broadcast_message<server_message_type::lobby_add_user>(lobby, p->second.user_id, p->second.name, p->second.profile_image);
    }
}

void game_manager::HANDLE_MESSAGE(game_start, user_ptr user) {
    if (user->second.in_lobby == lobby_ptr{}) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }

    auto &lobby = user->second.in_lobby->second;

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
    send_lobby_update(user->second.in_lobby);

    lobby.start_game(*this, all_cards);
}

void game_manager::HANDLE_MESSAGE(game_action, user_ptr user, const game_action &value) {
#ifdef DEBUG_PRINT_GAME_UPDATES
    std::cout << "/*** GAME ACTION *** ID = " << user->second.user_id << " ***/ " << json::serialize(value) << '\n';
#endif
    if (user->second.in_lobby == lobby_ptr{}) {
        throw lobby_error("ERROR_PLAYER_NOT_IN_LOBBY");
    }
    auto &lobby = user->second.in_lobby->second;

    if (lobby.state != lobby_state::playing) {
        throw lobby_error("ERROR_LOBBY_NOT_PLAYING");
    }

    if (auto it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); it != lobby.game.m_players.end()) {
        enums::visit_indexed([&](enums::enum_tag_for<game_action_type> auto tag, auto && ... args) {
            it->handle_action(tag, std::forward<decltype(args)>(args) ...);
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

void lobby::start_game(game_manager &mgr, const banggame::all_cards_t &all_cards) {
    game_options opts;
    opts.expansions = expansions;
    opts.keep_last_card_shuffling = false;

    mgr.broadcast_message<server_message_type::game_started>(*this, opts);

    game = {};
    
    std::vector<player *> ids;
    for (const auto &_ : users) {
        ids.push_back(&game.m_players.emplace(&game, game.m_players.first_available_id()));
    }
    std::ranges::shuffle(ids, game.rng);

    auto it = users.begin();
    for (player *p : ids) {
        p->user_id = (*it)->second.user_id;
        game.add_update<game_update_type::player_add>(p->id, (*it)->second.user_id);
        ++it;
    }

    game.start_game(opts, all_cards);
}