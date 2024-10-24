#include "chat_commands.h"

#include "manager.h"

#include "game/give_card.h"

#include "utils/parse_string.h"

namespace banggame {

    const string_command_map chat_command::commands {
        { "help",           { proxy<&game_manager::command_print_help>,         "HELP_DESCRIPTION" }},
        { "users",          { proxy<&game_manager::command_print_users>,        "USERS_DESCRIPTION" }},
        { "kick",           { proxy<&game_manager::command_kick_user>,          "KICK_DESCRIPTION", command_permissions::lobby_owner }},
        { "mute",           { proxy<&game_manager::command_mute_user>,          "MUTE_DESCRIPTION", command_permissions::lobby_owner }},
        { "unmute",         { proxy<&game_manager::command_unmute_user>,        "UNMUTE_DESCRIPTION", command_permissions::lobby_owner }},
        { "options",        { proxy<&game_manager::command_get_game_options>,   "GET_OPTIONS_DESCRIPTION" }},
        { "set-option",     { proxy<&game_manager::command_set_game_option>,    "SET_OPTION_DESCRIPTION", { command_permissions::lobby_owner, command_permissions::lobby_waiting } }},
        { "reset-options",  { proxy<&game_manager::command_reset_game_options>, "RESET_OPTIONS_DESCRIPTION", { command_permissions::lobby_owner, command_permissions::lobby_waiting } }},
        { "give",           { proxy<&game_manager::command_give_card>,          "GIVE_CARD_DESCRIPTION", command_permissions::game_cheat }},
        { "seed",           { proxy<&game_manager::command_get_rng_seed>,       "GET_RNG_SEED_DESCRIPTION", command_permissions::lobby_finished }},
        { "quit",           { proxy<&game_manager::command_quit>,               "QUIT_DESCRIPTION" }},
    };

    void game_manager::command_print_help(session_ptr session) {
        for (const auto &[cmd_name, command] : chat_command::commands) {
            if (!command.permissions().check(command_permissions::game_cheat) || m_options.enable_cheats) {
                send_message<"lobby_chat">(session->client, 0,
                    std::string{command.description()},
                    chat_format_arg_list{{utils::tag<"string">{}, std::format("{}{}", chat_command::start_char, cmd_name)}},
                    lobby_chat_flag::translated
                );
            }
        }
    }

    void game_manager::command_print_users(session_ptr session) {
        game_lobby &lobby = *session->lobby;
        for (const game_user &user : lobby.connected_users()) {
            if (user.flags.empty()) {
                send_message<"lobby_chat">(session->client, 0,
                    std::format("{} : {}", user.user_id, user.session->username)
                );
            } else {
                send_message<"lobby_chat">(session->client, 0,
                    std::format("{} : {} ({})", user.user_id, user.session->username, user.flags)
                );
            }
        }
    }

    void game_manager::command_kick_user(session_ptr session, std::string_view name_or_id) {
        game_user &user = session->lobby->find_user(name_or_id);
        kick_user_from_lobby(user.session);
    }

    void game_manager::command_mute_user(session_ptr session, std::string_view name_or_id) {
        game_user &user = session->lobby->find_user(name_or_id);
        add_user_flag(session, game_user_flag::muted);
    }

    void game_manager::command_unmute_user(session_ptr session, std::string_view name_or_id) {
        game_user &user = session->lobby->find_user(name_or_id);
        remove_user_flag(session, game_user_flag::muted);
    }
    
    void game_manager::command_get_game_options(session_ptr session) {
        send_message<"lobby_chat">(session->client, 0, session->lobby->options.to_string());
    }

    void game_manager::command_set_game_option(session_ptr session, std::string_view key, std::string_view value) {
        try {
            game_lobby &lobby = *session->lobby;
            lobby.options.set_option(key, value);
            broadcast_message_lobby<"lobby_game_options">(lobby, lobby.options);
        } catch (const std::exception &e) {
            throw lobby_error(e.what());
        }
    }

    void game_manager::command_reset_game_options(session_ptr session) {
        game_lobby &lobby = *session->lobby;
        lobby.options = game_options{};
        broadcast_message_lobby<"lobby_game_options">(lobby, lobby.options);
    }

    void game_manager::command_give_card(session_ptr session, std::string_view card_name) {
        game_lobby &lobby = *session->lobby;
        game_user &user = lobby.find_user(session);

        player_ptr target = lobby.m_game->find_player_by_userid(user.user_id);
        if (!target) {
            throw lobby_error("ERROR_USER_NOT_CONTROLLING_PLAYER");
        }

        if (lobby.m_game->pending_requests() || lobby.m_game->is_waiting() || lobby.m_game->m_playing != target) {
            throw lobby_error("ERROR_PLAYER_NOT_IN_TURN");
        }

        if (!give_card(target, card_name)) {
            throw lobby_error("ERROR_CANNOT_FIND_CARD");
        }
    }

    void game_manager::command_get_rng_seed(session_ptr session) {
        send_message<"lobby_chat">(session->client, 0,
            "GAME_SEED", chat_format_arg_list{{utils::tag<"integer">{}, session->lobby->m_game->rng_seed}}, lobby_chat_flag::translated
        );
    }

    void game_manager::command_quit(session_ptr session) {
        kick_client(session->client, "QUIT");
    }

}