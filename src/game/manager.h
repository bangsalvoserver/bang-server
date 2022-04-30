#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <json/json.h>
#include <thread>

#include "utils/tsqueue.h"

#include "net_options.h"
#include "net_enums.h"

#include "game.h"

namespace banggame {

class game_manager;
class game_user;

using user_map = std::map<int, game_user>;
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
    std::string name;
    std::vector<std::byte> profile_image;
    lobby_ptr in_lobby{};
};

struct client_message_pair {
    int client_id;
    client_message value;
};

template<server_message_type E, typename ... Ts>
server_message make_message(Ts && ... args) {
    return server_message{enums::enum_tag<E>, std::forward<Ts>(args) ...};
}

#define MESSAGE_TAG(name) enums::enum_tag_t<banggame::client_message_type::name>
#define HANDLE_MESSAGE(name, ...) handle_message(MESSAGE_TAG(name) __VA_OPT__(,) __VA_ARGS__)

using send_message_function = std::function<void(int, server_message)>;

class game_manager {
public:
    void on_receive_message(int client_id, client_message &&msg) {
        m_in_queue.emplace_back(client_id, std::move(msg));
    }
    
    void handle_message(int client_id, const client_message &msg);

    void client_disconnected(int client_id);
    bool client_validated(int client_id) const;

    template<server_message_type E, typename ... Ts>
    void send_message(int client_id, Ts && ... args) {
        m_send_message(client_id, make_message<E>(std::forward<Ts>(args) ... ));
    }

    void set_send_message_function(send_message_function &&fun) {
        m_send_message = std::move(fun);
    }

    template<server_message_type E, typename ... Ts>
    void broadcast_message(const lobby &lobby, Ts && ... args) {
        auto msg = make_message<E>(std::forward<Ts>(args) ... );
        for (user_ptr it : lobby.users) {
            m_send_message(it->first, msg);
        }
    }

    void start(std::stop_token token);

private:
    lobby_data make_lobby_data(lobby_ptr it);
    void send_lobby_update(lobby_ptr it);

    void HANDLE_MESSAGE(connect,        int client_id, const connect_args &value);
    void HANDLE_MESSAGE(lobby_list,     user_ptr user);
    void HANDLE_MESSAGE(lobby_make,     user_ptr user, const lobby_info &value);
    void HANDLE_MESSAGE(lobby_edit,     user_ptr user, const lobby_info &args);
    void HANDLE_MESSAGE(lobby_join,     user_ptr user, const lobby_join_args &value);
    void HANDLE_MESSAGE(lobby_leave,    user_ptr user);
    void HANDLE_MESSAGE(lobby_chat,     user_ptr user, const lobby_chat_client_args &value);
    void HANDLE_MESSAGE(lobby_return,   user_ptr user);
    void HANDLE_MESSAGE(game_start,     user_ptr user);
    void HANDLE_MESSAGE(game_action,    user_ptr user, const banggame::game_action &value);

    std::map<int, game_user> users;
    lobby_map m_lobbies;

    int m_lobby_counter = 0;

    util::tsqueue<client_message_pair> m_in_queue;
    
    send_message_function m_send_message;

    banggame::all_cards_t all_cards;
};

}

#endif