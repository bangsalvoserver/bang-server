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
    void tick();

    void send_lobby_update(const lobby &lobby);

    template<server_message_type E>
    void send_message(client_handle client, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        if (m_options.verbose) {
            fmt::print("{}: Sent {}\n", get_client_ip(client), message);
            fflush(stdout);
        }
        push_message(client, message);
    }

    template<server_message_type E>
    void broadcast_message(auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        if (m_options.verbose) {
            fmt::print("All users: Sent {}\n", message);
            fflush(stdout);
        }
        for (game_user &user : m_users | rv::values) {
            push_message(user.client, message);
        }
    }

    template<server_message_type E>
    void broadcast_message_lobby(const lobby &lobby, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        if (m_options.verbose) {
            fmt::print("Lobby {}: Sent {}\n", lobby.name, message);
            fflush(stdout);
        }
        for (auto &[team, user_id, u] : lobby.users) {
            push_message(u->client, message);
        }
    }

    server_options &options() { return m_options; }

protected:
    void on_connect(client_handle client) override;
    void on_disconnect(client_handle client) override;
    void on_message(client_handle client, const std::string &message) override;

private:
    void kick_user_from_lobby(game_user &user);
    void handle_join_lobby(game_user &user, lobby &lobby);

private:
    std::string handle_message(MSG_TAG(connect),        client_handle client, const connect_args &value);
    std::string handle_message(MSG_TAG(pong),           client_handle client);
    std::string handle_message(MSG_TAG(user_edit),      game_user &user, const user_info &value);
    std::string handle_message(MSG_TAG(lobby_make),     game_user &user, const lobby_info &value);
    std::string handle_message(MSG_TAG(lobby_edit),     game_user &user, const lobby_info &args);
    std::string handle_message(MSG_TAG(lobby_join),     game_user &user, const lobby_id_args &value);
    std::string handle_message(MSG_TAG(lobby_leave),    game_user &user);
    std::string handle_message(MSG_TAG(lobby_chat),     game_user &user, const lobby_chat_client_args &value);
    std::string handle_message(MSG_TAG(lobby_return),   game_user &user);
    std::string handle_message(MSG_TAG(game_start),     game_user &user);
    std::string handle_message(MSG_TAG(game_rejoin),    game_user &user, int player_id);
    std::string handle_message(MSG_TAG(game_action),    game_user &user, const json::json &value);

    std::string handle_chat_command(game_user &user, const std::string &command);

    std::string command_print_help(game_user &user);
    std::string command_print_users(game_user &user);
    std::string command_kick_user(game_user &user, std::string_view user_id_str);
    std::string command_get_game_options(game_user &user);
    std::string command_set_game_option(game_user &user, std::string_view name, std::string_view value);
    std::string command_give_card(game_user &user, std::string_view name);
    std::string command_set_team(game_user &user, std::string_view value);
    std::string command_get_rng_seed(game_user &user);
    std::string command_quit(game_user &user);

private:
    user_map m_users;
    lobby_map m_lobbies;
    lobby_list m_lobby_order;
    client_map m_clients;

    server_options m_options;

    id_type m_lobby_count = 0;

    friend struct lobby;
    friend class chat_command;
};

}

#endif