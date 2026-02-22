#include "manager.h"
#include "server_options.h"

#include "utils/parse_string.h"

namespace banggame {

    void game_manager::register_chat_commands(game_lobby &lobby) {
        lobby.add_command("help", "HELP_DESCRIPTION", {}, [&](session_ptr session) {
            for (const auto &command : lobby.m_commands) {
                if (!command.permissions.check(command_permissions::game_cheat) || m_options.enable_cheats) {
                    send_message(session->client, server_messages::lobby_chat{ 0,
                        std::string{command.description},
                        {chat_format_arg::string{std::format("{}{}", chat_command_start_char, command.name)}},
                        lobby_chat_flag::translated
                    });
                }
            }
        });

        lobby.add_command("users", "USERS_DESCRIPTION", {}, [&](session_ptr session) {
            for (const game_user &user : lobby.connected_users()) {
                if (user.flags.empty()) {
                    send_message(session->client, server_messages::lobby_chat{ 0,
                        std::format("{} : {}", user.user_id, user.session->username)
                    });
                } else {
                    send_message(session->client, server_messages::lobby_chat{ 0,
                        std::format("{} : {} ({})", user.user_id, user.session->username, user.flags)
                    });
                }
            }
        });

        lobby.add_command("kick", "KICK_DESCRIPTION", command_permissions::lobby_owner, [&](session_ptr session, std::string_view name_or_id) {
            game_user &user = lobby.find_user(name_or_id);
            kick_user_from_lobby(user.session);
        });

        lobby.add_command("mute", "MUTE_DESCRIPTION", command_permissions::lobby_owner, [&](session_ptr session, std::string_view name_or_id) {
            add_user_flag(lobby, lobby.find_user(name_or_id), game_user_flag::muted);
        });
        
        lobby.add_command("unmute", "UNMUTE_DESCRIPTION", command_permissions::lobby_owner, [&](session_ptr session, std::string_view name_or_id) {
            remove_user_flag(lobby, lobby.find_user(name_or_id), game_user_flag::muted);
        });

        lobby.add_command("appoint", "APPOINT_DESCRIPTION", command_permissions::lobby_owner, [&](session_ptr session, std::string_view name_or_id) {
            if (add_user_flag(lobby, lobby.find_user(name_or_id), game_user_flag::lobby_owner)) {
                remove_user_flag(lobby, lobby.find_user(session), game_user_flag::lobby_owner);
            }
        });

        lobby.add_command("options", "GET_OPTIONS_DESCRIPTION", {}, [&](session_ptr session) {
            send_message(session->client, server_messages::lobby_chat{ 0, session->lobby->options.to_string() });
        });

        lobby.add_command("set-option", "SET_OPTION_DESCRIPTION", {}, [&](session_ptr session, std::string_view key, std::string_view value) {
            try {
                lobby.options.set_option(key, value);
                broadcast_message_lobby(lobby, server_messages::lobby_game_options{ lobby.options });
            } catch (const game_option_error &e) {
                throw lobby_error(e.what());
            }
        });

        lobby.add_command("reset-options", "RESET_OPTIONS_DESCRIPTION", { command_permissions::lobby_owner, command_permissions::lobby_waiting }, [&](session_ptr session) {
            lobby.options = game_options{};
            broadcast_message_lobby(lobby, server_messages::lobby_game_options{ lobby.options });
        });

        lobby.add_command("quit", "QUIT_DESCRIPTION", {}, [&](session_ptr session) {
            kick_client(session->client, "QUIT");
        });
    };

}