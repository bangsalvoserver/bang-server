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
    void broadcast_message(auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        for (game_session &session : m_sessions | rv::values) {
            push_message(session.client, message);
        }
    }

    template<utils::fixed_string E> requires server_message_type<E>
    void broadcast_message_lobby(const game_lobby &lobby, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        for (const game_user &user : lobby.users) {
            push_message(user.session.client, message);
        }
    }

    void invalidate_connection(client_handle client);
    void kick_user_from_lobby(game_session &session);
    void add_lobby_chat_message(game_lobby &lobby, game_user *is_read_for, lobby_chat_args message);
    void handle_join_lobby(game_session &session, game_lobby &lobby);
    void set_user_team(game_session &session, lobby_team team);

private:
    void handle_message(utils::tag<"connect">,        client_handle client, connection &con, const connect_args &value);
    void handle_message(utils::tag<"pong">,           client_handle client, connection &con);
    void handle_message(utils::tag<"user_set_name">,  game_session &session, const std::string &username);
    void handle_message(utils::tag<"user_set_propic">, game_session &session, const image_pixels &propic);
    void handle_message(utils::tag<"lobby_make">,     game_session &session, const lobby_make_args &value);
    void handle_message(utils::tag<"lobby_edit">,     game_session &session, const lobby_info &args);
    void handle_message(utils::tag<"lobby_join">,     game_session &session, const lobby_join_args &value);
    void handle_message(utils::tag<"lobby_leave">,    game_session &session);
    void handle_message(utils::tag<"lobby_chat">,     game_session &session, const lobby_chat_client_args &value);
    void handle_message(utils::tag<"lobby_return">,   game_session &session);
    void handle_message(utils::tag<"user_set_team">,  game_session &session, lobby_team team);
    void handle_message(utils::tag<"game_start">,     game_session &session);
    void handle_message(utils::tag<"game_rejoin">,    game_session &session, const game_rejoin_args &value);
    void handle_message(utils::tag<"game_action">,    game_session &session, const json::json &value);

    void handle_chat_command(game_session &session, const std::string &command);

    void command_print_help(game_session &session);
    void command_print_users(game_session &session);
    void command_kick_user(game_session &session, std::string_view target_user);
    void command_mute_user(game_session &session, std::string_view target_user);
    void command_unmute_user(game_session &session, std::string_view target_user);
    void command_get_game_options(game_session &session);
    void command_set_game_option(game_session &session, std::string_view key, std::string_view value);
    void command_reset_game_options(game_session &session);
    void command_give_card(game_session &session, std::string_view card_name);
    void command_get_rng_seed(game_session &session);
    void command_quit(game_session &session);

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