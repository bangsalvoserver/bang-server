#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "lobby.h"
#include "chat_commands.h"
#include "wsserver.h"
#include "logging.h"

#include <random>

namespace banggame {

template<utils::fixed_string E> requires server_message_type<E>
std::string make_message(auto && ... args) {
    return serialize_message(server_message{utils::tag<E>{}, FWD(args) ...})
        .dump(-1, ' ', true, nlohmann::json::error_handler_t::replace);
}

struct server_options {
    bool enable_cheats = false;
    int max_session_id_count = 10;
};

class game_manager: public net::wsserver {
public:
    game_manager();

    void stop();
    void tick();
    void kick_client(client_handle client, std::string message, int code = kick_opcode);

    server_options &options() { return m_options; }

protected:
    void on_connect(client_handle client) override;
    void on_disconnect(client_handle client) override;
    void on_message(client_handle client, std::string_view message) override;

private:
    template<utils::fixed_string E> requires server_message_type<E>
    void send_message(client_handle client, auto && ... args) {
        push_message(client, make_message<E>(FWD(args) ... ));
    }

    template<utils::fixed_string E> requires server_message_type<E>
    void broadcast_message_no_lobby(auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        for (session_ptr session : m_sessions | rv::values) {
            if (!session->lobby) {
                push_message(session->client, message);
            }
        }
    }

    template<utils::fixed_string E> requires server_message_type<E>
    void broadcast_message_lobby(const game_lobby &lobby, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        for (const game_user &user : lobby.connected_users()) {
            push_message(user.session->client, message);
        }
    }

    void invalidate_connection(client_handle client);
    void kick_user_from_lobby(session_ptr session);
    void add_lobby_chat_message(game_lobby &lobby, game_user *is_read_for, lobby_chat_args message);
    void handle_join_lobby(session_ptr session, game_lobby &lobby);

    bool add_user_flag(session_ptr session, game_user_flag flag);
    bool remove_user_flag(session_ptr session, game_user_flag flag);

private:
    void handle_message(utils::tag<"connect">,        client_handle client, connection &con, connect_args value);
    void handle_message(utils::tag<"pong">,           client_handle client, connection &con);
    void handle_message(utils::tag<"user_set_name">,  session_ptr session, std::string username);
    void handle_message(utils::tag<"user_set_propic">, session_ptr session, image_pixels propic);
    void handle_message(utils::tag<"lobby_make">,     session_ptr session, const lobby_make_args &value);
    void handle_message(utils::tag<"lobby_game_options">, session_ptr session, const game_options &options);
    void handle_message(utils::tag<"lobby_join">,     session_ptr session, const lobby_join_args &value);
    void handle_message(utils::tag<"lobby_leave">,    session_ptr session);
    void handle_message(utils::tag<"lobby_chat">,     session_ptr session, const lobby_chat_client_args &value);
    void handle_message(utils::tag<"lobby_return">,   session_ptr session);
    void handle_message(utils::tag<"user_spectate">,  session_ptr session, bool spectator);
    void handle_message(utils::tag<"game_start">,     session_ptr session);
    void handle_message(utils::tag<"game_rejoin">,    session_ptr session, const game_rejoin_args &value);
    void handle_message(utils::tag<"game_action">,    session_ptr session, const json::json &value);

    void handle_chat_command(session_ptr session, const std::string &command);

    void command_print_help(session_ptr session);
    void command_print_users(session_ptr session);
    void command_kick_user(session_ptr session, std::string_view target_user);
    void command_mute_user(session_ptr session, std::string_view target_user);
    void command_unmute_user(session_ptr session, std::string_view target_user);
    void command_get_game_options(session_ptr session);
    void command_set_game_option(session_ptr session, std::string_view key, std::string_view value);
    void command_reset_game_options(session_ptr session);
    void command_give_card(session_ptr session, std::string_view card_name);
    void command_get_rng_seed(session_ptr session);
    void command_quit(session_ptr session);

private:
    std::default_random_engine session_rng;

    user_map m_sessions;
    lobby_map m_lobbies;
    client_map m_connections;

    server_options m_options;

    friend class chat_command;
};

}

#endif