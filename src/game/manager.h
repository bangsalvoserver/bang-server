#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <json/json.h>
#include <thread>

#include "utils/tsqueue.h"

#include "net_options.h"
#include "messages.h"

#include "game.h"

namespace banggame {

class game_manager;
class game_user;

using client_handle = std::weak_ptr<void>;
using user_map = std::map<client_handle, game_user, std::owner_less<client_handle>>;
using user_ptr = user_map::iterator;

struct lobby : lobby_info {
    std::vector<user_ptr> users;
    user_ptr owner;
    lobby_state state;

    banggame::game game;
    void start_game(game_manager &mgr, const banggame::all_cards_t &all_cards);
    void send_updates(game_manager &mgr);
};

using lobby_map = std::map<int, lobby>;
using lobby_ptr = lobby_map::iterator;

struct game_user {
    int user_id;
    std::string name;
    sdl::image_pixels profile_image;
    lobby_ptr in_lobby{};
};

template<server_message_type E, typename ... Ts>
server_message make_message(Ts && ... args) {
    return server_message{enums::enum_tag<E>, std::forward<Ts>(args) ...};
}

#define MESSAGE_TAG(name) enums::enum_tag_t<banggame::client_message_type::name>
#define HANDLE_MESSAGE(name, ...) handle_message(MESSAGE_TAG(name) __VA_OPT__(,) __VA_ARGS__)

using send_message_function = std::function<void(client_handle, server_message)>;
using print_error_function = std::function<void(const std::string &message)>;

class game_manager {
public:
    void set_send_message_function(send_message_function &&fun) {
        m_send_message = std::move(fun);
    }

    void set_print_error_function(print_error_function &&fun) {
        m_print_error = std::move(fun);
    }

    void on_receive_message(client_handle client, client_message &&msg) {
        m_in_queue.emplace_back(client, std::move(msg));
    }

    void client_disconnected(client_handle client);
    
    bool client_validated(client_handle client) const;

    void start(std::stop_token token);

private:
    lobby_data make_lobby_data(lobby_ptr it);
    void send_lobby_update(lobby_ptr it);

    void handle_message(client_handle client, const client_message &msg);

    template<server_message_type E, typename ... Ts>
    void send_message(client_handle client, Ts && ... args) {
        m_send_message(client, make_message<E>(std::forward<Ts>(args) ... ));
    }

    template<server_message_type E, typename ... Ts>
    void broadcast_message(const lobby &lobby, Ts && ... args) {
        auto msg = make_message<E>(std::forward<Ts>(args) ... );
        for (user_ptr it : lobby.users) {
            m_send_message(it->first, msg);
        }
    }

    void print_error(const std::string &message) {
        if (m_print_error) {
            m_print_error(message);
        }
    }

    void HANDLE_MESSAGE(connect,        client_handle client, const connect_args &value);
    void HANDLE_MESSAGE(lobby_list,     user_ptr user);
    void HANDLE_MESSAGE(lobby_make,     user_ptr user, const lobby_info &value);
    void HANDLE_MESSAGE(lobby_edit,     user_ptr user, const lobby_info &args);
    void HANDLE_MESSAGE(lobby_join,     user_ptr user, const lobby_join_args &value);
    void HANDLE_MESSAGE(lobby_leave,    user_ptr user);
    void HANDLE_MESSAGE(lobby_chat,     user_ptr user, const lobby_chat_client_args &value);
    void HANDLE_MESSAGE(lobby_return,   user_ptr user);
    void HANDLE_MESSAGE(game_start,     user_ptr user);
    void HANDLE_MESSAGE(game_action,    user_ptr user, const banggame::game_action &value);

private:
    user_map users;
    lobby_map m_lobbies;

    int m_lobby_counter = 0;
    int m_user_counter = 0;

    util::tsqueue<std::pair<client_handle, client_message>> m_in_queue;
    
    send_message_function m_send_message;
    print_error_function m_print_error;

    banggame::all_cards_t all_cards;

    friend class lobby;
};

}

#endif