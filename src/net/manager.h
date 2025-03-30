#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "lobby.h"
#include "chat_commands.h"
#include "wsserver.h"
#include "logging.h"

#include <random>

namespace banggame {

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
    void send_message(client_handle client, const server_message &msg) {
        m_outgoing_messages.emplace_back(client, serialize_message(msg));
    }

    void broadcast_message_no_lobby(const server_message &msg) {
        std::string message = serialize_message(msg);
        for (session_ptr session : m_sessions | rv::values) {
            if (!session->lobby) {
                m_outgoing_messages.emplace_back(session->client, message);
            }
        }
    }

    void broadcast_message_lobby(const game_lobby &lobby, const server_message &msg) {
        std::string message = serialize_message(msg);
        for (const game_user &user : lobby.connected_users()) {
            m_outgoing_messages.emplace_back(user.session->client, message);
        }
    }

    void invalidate_connection(client_handle client);
    void kick_user_from_lobby(session_ptr session);
    void add_lobby_chat_message(game_lobby &lobby, game_user *is_read_for, server_messages::lobby_chat message);
    void handle_join_lobby(session_ptr session, game_lobby &lobby);

    bool add_user_flag(game_lobby &lobby, game_user &user, game_user_flag flag);
    bool remove_user_flag(game_lobby &lobby, game_user &user, game_user_flag flag);

private:
    void handle_message(client_messages::connect &&msg, client_handle client, connection &con);
    void handle_message(client_messages::pong &&msg, client_handle client, connection &con);
    void handle_message(client_messages::user_set_name &&msg, session_ptr session);
    void handle_message(client_messages::user_set_propic &&msg, session_ptr session);
    void handle_message(client_messages::lobby_make &&msg, session_ptr session);
    void handle_message(client_messages::lobby_game_options &&msg, session_ptr session);
    void handle_message(client_messages::lobby_join &&msg, session_ptr session);
    void handle_message(client_messages::lobby_leave &&msg, session_ptr session);
    void handle_message(client_messages::lobby_chat &&msg, session_ptr session);
    void handle_message(client_messages::lobby_return &&msg, session_ptr session);
    void handle_message(client_messages::user_spectate &&msg, session_ptr session);
    void handle_message(client_messages::game_start &&msg, session_ptr session);
    void handle_message(client_messages::game_rejoin &&msg, session_ptr session);
    void handle_message(client_messages::game_action &&msg, session_ptr session);

    void handle_chat_command(session_ptr session, const std::string &command);

    void command_print_help(session_ptr session);
    void command_print_users(session_ptr session);
    void command_kick_user(session_ptr session, std::string_view target_user);
    void command_mute_user(session_ptr session, std::string_view target_user);
    void command_unmute_user(session_ptr session, std::string_view target_user);
    void command_appoint_user(session_ptr session, std::string_view target_user);
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

    message_list m_outgoing_messages;

    server_options m_options;

    friend class chat_command;
};

}

#endif