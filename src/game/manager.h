#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <json/json.h>

#include "net_options.h"
#include "messages.h"

#include "game.h"

namespace banggame {

class game_manager;
struct game_user;

using client_handle = std::weak_ptr<void>;
using user_map = std::map<client_handle, game_user, std::owner_less<client_handle>>;
using user_ptr = user_map::iterator;

struct lobby : lobby_info {
    static constexpr int lifetime_seconds = 10;

    std::vector<user_ptr> users;
    lobby_state state;
    int lifetime;

    banggame::game game;
    void start_game(game_manager &mgr);
    void send_updates(game_manager &mgr);
};

using lobby_map = std::map<int, lobby>;
using lobby_ptr = lobby_map::iterator;

struct game_user {
    int user_id;
    std::string name;
    sdl::image_pixels profile_image;
    std::optional<lobby_ptr> in_lobby;
};

template<server_message_type E>
server_message make_message(auto && ... args) {
    return server_message{enums::enum_tag<E>, FWD(args) ...};
}

using send_message_function = std::function<void(client_handle, server_message)>;
using print_error_function = std::function<void(const std::string &message)>;
using kick_client_function = std::function<void(client_handle, const std::string &message)>;

class game_manager {
public:
    void set_send_message_function(send_message_function &&fun) {
        m_send_message = std::move(fun);
    }

    void set_print_error_function(print_error_function &&fun) {
        m_print_error = std::move(fun);
    }

    void set_kick_client_function(kick_client_function &&fun) {
        m_kick_client = std::move(fun);
    }

    void on_receive_message(client_handle client, const client_message &msg);

    void client_disconnected(client_handle client);
    
    bool client_validated(client_handle client) const;

    void tick();

private:
    lobby_data make_lobby_data(lobby_ptr it);
    void send_lobby_update(lobby_ptr it);

    template<server_message_type E>
    void send_message(client_handle client, auto && ... args) {
        m_send_message(client, make_message<E>(FWD(args) ... ));
    }

    template<server_message_type E>
    void broadcast_message(auto && ... args) {
        auto msg = make_message<E>(FWD(args) ... );
        for (client_handle client : users | std::views::keys) {
            m_send_message(client, msg);
        }
    }

    template<server_message_type E>
    void broadcast_message_lobby(const lobby &lobby, auto && ... args) {
        auto msg = make_message<E>(FWD(args) ... );
        for (user_ptr it : lobby.users) {
            m_send_message(it->first, msg);
        }
    }

    void print_error(const std::string &message) {
        if (m_print_error) {
            m_print_error(message);
        }
    }

    void kick_client(client_handle client, const std::string &message) {
        if (m_kick_client) {
            m_kick_client(client, message);
        }
    }

    std::string handle_message(MSG_TAG(connect),        client_handle client, const connect_args &value);
    std::string handle_message(MSG_TAG(lobby_list),     user_ptr user);
    std::string handle_message(MSG_TAG(lobby_make),     user_ptr user, const lobby_info &value);
    std::string handle_message(MSG_TAG(lobby_edit),     user_ptr user, const lobby_info &args);
    std::string handle_message(MSG_TAG(lobby_join),     user_ptr user, const lobby_id_args &value);
    std::string handle_message(MSG_TAG(lobby_rejoin),   user_ptr user, const lobby_rejoin_args &value);
    std::string handle_message(MSG_TAG(lobby_leave),    user_ptr user);
    std::string handle_message(MSG_TAG(lobby_chat),     user_ptr user, const lobby_chat_client_args &value);
    std::string handle_message(MSG_TAG(lobby_return),   user_ptr user);
    std::string handle_message(MSG_TAG(game_start),     user_ptr user);
    std::string handle_message(MSG_TAG(game_action),    user_ptr user, const banggame::game_action &value);

private:
    user_map users;
    lobby_map m_lobbies;

    int m_lobby_counter = 0;
    int m_user_counter = 0;
    
    send_message_function m_send_message;
    print_error_function m_print_error;
    kick_client_function m_kick_client;

    friend struct lobby;
};

}

#endif