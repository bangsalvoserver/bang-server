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

    void tick();

    server_options &options() { return m_options; }

protected:
    void on_connect(client_handle client) override;
    void on_disconnect(client_handle client) override;
    void on_message(client_handle client, const std::string &message) override;

private:
    template<utils::fixed_string E> requires server_message_type<E>
    void send_message(client_handle client, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        logging::info("{}: Sent {}", get_client_ip(client), message);
        push_message(client, message);
    }

    template<utils::fixed_string E> requires server_message_type<E>
    void broadcast_message(auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        logging::info("All users: Sent {}", message);
        for (game_user &user : m_users | rv::values) {
            push_message(user.client, message);
        }
    }

    template<utils::fixed_string E> requires server_message_type<E>
    void broadcast_message_lobby(const lobby &lobby, auto && ... args) {
        std::string message = make_message<E>(FWD(args) ... );
        logging::info("Lobby {}: Sent {}", lobby.name, message);
        for (const lobby_user &user : lobby.users) {
            push_message(user.user->client, message);
        }
    }

    void kick_user_from_lobby(game_user &user);
    void handle_join_lobby(game_user &user, lobby &lobby);

    void set_user_team(game_user &user, lobby_team team);

private:
    void handle_message(utils::tag<"connect">,        client_state &state, const connect_args &value);
    void handle_message(utils::tag<"pong">,           client_state &state);
    void handle_message(utils::tag<"user_set_name">,  game_user &user, const std::string &username);
    void handle_message(utils::tag<"user_set_propic">, game_user &user, const utils::image_pixels &propic);
    void handle_message(utils::tag<"lobby_make">,     game_user &user, const lobby_info &value);
    void handle_message(utils::tag<"lobby_edit">,     game_user &user, const lobby_info &args);
    void handle_message(utils::tag<"lobby_join">,     game_user &user, const lobby_id_args &value);
    void handle_message(utils::tag<"lobby_leave">,    game_user &user);
    void handle_message(utils::tag<"lobby_chat">,     game_user &user, const lobby_chat_client_args &value);
    void handle_message(utils::tag<"lobby_return">,   game_user &user);
    void handle_message(utils::tag<"user_set_team">,  game_user &user, lobby_team team);
    void handle_message(utils::tag<"game_start">,     game_user &user);
    void handle_message(utils::tag<"game_rejoin">,    game_user &user, const game_rejoin_args &value);
    void handle_message(utils::tag<"game_action">,    game_user &user, const json::json &value);

    void handle_chat_command(game_user &user, const std::string &command);

    void command_print_help(game_user &user);
    void command_print_users(game_user &user);
    void command_kick_user(game_user &user, std::string_view target_user);
    void command_mute_user(game_user &user, std::string_view target_user);
    void command_unmute_user(game_user &user, std::string_view target_user);
    void command_get_game_options(game_user &user);
    void command_reset_game_options(game_user &user);
    void command_give_card(game_user &user, std::string_view name);
    void command_get_rng_seed(game_user &user);
    void command_quit(game_user &user);

private:
    std::default_random_engine session_rng;

    user_map m_users;
    lobby_map m_lobbies;
    lobby_list m_lobby_order;
    client_map m_clients;

    server_options m_options;

    id_type m_lobby_count = 0;

    friend class chat_command;
};

}

#endif