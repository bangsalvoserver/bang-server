#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "lobby.h"
#include "chat_commands.h"
#include "wsserver.h"

#include <random>

namespace banggame {

template<server_message_type E>
std::string make_message(auto && ... args) {
    return json::serialize(server_message{enums::enum_tag<E>, FWD(args) ...}).dump();
}

struct server_options {
    bool enable_cheats = false;
    bool verbose = false;
};

class game_manager: public net::wsserver {
public:
    game_manager(asio::io_context &ctx);
    
    bool client_validated(client_handle client) const;

    void tick();

    void send_lobby_update(const lobby &lobby);

    template<server_message_type E>
    void send_message(client_handle client, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        if (m_options.verbose) {
            std::cout << get_client_ip(client) << ": Sent " << message << std::endl;
        }
        push_message(client, message);
    }

    template<server_message_type E>
    void broadcast_message(auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        if (m_options.verbose) {
            std::cout << "All users: Sent " << message << std::endl;
        }
        for (client_handle client : users | std::views::keys) {
            push_message(client, message);
        }
    }

    template<server_message_type E>
    void broadcast_message_lobby(const lobby &lobby, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        if (m_options.verbose) {
            std::cout << "Lobby " << lobby.name << ": Sent " << message << std::endl;
        }
        for (auto [team, it] : lobby.users) {
            push_message(it->first, message);
        }
    }

    server_options &options() { return m_options; }

protected:
    void on_disconnect(client_handle client) override;
    void on_message(client_handle client, const std::string &message) override;

private:
    void kick_user_from_lobby(user_ptr user);

    std::string handle_message(MSG_TAG(connect),        client_handle client, const connect_args &value);
    std::string handle_message(MSG_TAG(pong),           user_ptr user);
    std::string handle_message(MSG_TAG(user_edit),      user_ptr user, const user_info &value);
    std::string handle_message(MSG_TAG(lobby_list),     user_ptr user);
    std::string handle_message(MSG_TAG(lobby_make),     user_ptr user, const lobby_info &value);
    std::string handle_message(MSG_TAG(lobby_edit),     user_ptr user, const lobby_info &args);
    std::string handle_message(MSG_TAG(lobby_join),     user_ptr user, const lobby_id_args &value);
    std::string handle_message(MSG_TAG(lobby_leave),    user_ptr user);
    std::string handle_message(MSG_TAG(lobby_chat),     user_ptr user, const lobby_chat_client_args &value);
    std::string handle_message(MSG_TAG(lobby_return),   user_ptr user);
    std::string handle_message(MSG_TAG(game_start),     user_ptr user);
    std::string handle_message(MSG_TAG(game_action),    user_ptr user, const json::json &value);

    std::string handle_chat_command(user_ptr user, const std::string &command);

    std::string command_print_help(user_ptr user);
    std::string command_print_users(user_ptr user);
    std::string command_kick_user(user_ptr user, std::string_view userid);
    std::string command_get_game_options(user_ptr user);
    std::string command_set_game_option(user_ptr user, std::string_view name, std::string_view value);
    std::string command_give_card(user_ptr user, std::string_view name);
    std::string command_set_team(user_ptr user, std::string_view value);
    std::string command_get_rng_seed(user_ptr user);

private:
    user_map users;
    lobby_list m_lobbies;

    server_options m_options;

    int generate_lobby_id();
    int generate_user_id(int user_id);

    std::default_random_engine m_rng;

    friend struct lobby;
    friend class chat_command;
};

}

#endif