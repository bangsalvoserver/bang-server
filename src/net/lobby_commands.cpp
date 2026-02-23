#include "manager.h"
#include "server_options.h"

#include "utils/parse_string.h"

namespace banggame {

    void game_manager::register_chat_commands(game_lobby &lobby) {
        lobby.add_command("help", "HELP_DESCRIPTION", {}, [](game_lobby &lobby, int user_id) {
            for (const auto &command : lobby.m_commands) {
                lobby.send_chat_message(user_id, { 0,
                    std::string{command.description},
                    {chat_format_arg::string{std::format("{}{}", chat_command::start_char, command.name)}},
                    lobby_chat_flag::translated
                });
            }
        });

        lobby.add_command("users", "USERS_DESCRIPTION", {}, [](game_lobby &lobby, int user_id) {
            for (const game_user &user : lobby.connected_users()) {
                if (user.flags.empty()) {
                    lobby.send_chat_message(user_id, { 0,
                        std::format("{} : {}", user.user_id, user.session->username)
                    });
                } else {
                    lobby.send_chat_message(user_id, { 0,
                        std::format("{} : {} ({})", user.user_id, user.session->username, user.flags)
                    });
                }
            }
        });

        lobby.add_command("kick", "KICK_DESCRIPTION", command_permissions::lobby_owner, [](game_lobby &lobby, int user_id, std::string_view name_or_id) {
            lobby.m_mgr->kick_user_from_lobby(lobby.find_user(name_or_id).session);
        });

        lobby.add_command("mute", "MUTE_DESCRIPTION", command_permissions::lobby_owner, [](game_lobby &lobby, int user_id, std::string_view name_or_id) {
            lobby.add_user_flag(lobby.find_user(name_or_id), game_user_flag::muted);
        });
        
        lobby.add_command("unmute", "UNMUTE_DESCRIPTION", command_permissions::lobby_owner, [](game_lobby &lobby, int user_id, std::string_view name_or_id) {
            lobby.remove_user_flag(lobby.find_user(name_or_id), game_user_flag::muted);
        });

        lobby.add_command("appoint", "APPOINT_DESCRIPTION", command_permissions::lobby_owner, [](game_lobby &lobby, int user_id, std::string_view name_or_id) {
            if (lobby.add_user_flag(lobby.find_user(name_or_id), game_user_flag::lobby_owner)) {
                lobby.remove_user_flag(lobby.find_user(user_id), game_user_flag::lobby_owner);
            }
        });

        lobby.add_command("options", "GET_OPTIONS_DESCRIPTION", {}, [](game_lobby &lobby, int user_id) {
            lobby.send_chat_message(user_id, { 0, lobby.options.to_string() });
        });

        lobby.add_command("set-option", "SET_OPTION_DESCRIPTION", {}, [](game_lobby &lobby, int user_id, std::string_view key, std::string_view value) {
            try {
                lobby.options.set_option(key, value);
                lobby.broadcast_message(server_messages::lobby_game_options{ lobby.options });
            } catch (const game_option_error &e) {
                throw lobby_error(e.what());
            }
        });

        lobby.add_command("reset-options", "RESET_OPTIONS_DESCRIPTION", { command_permissions::lobby_owner, command_permissions::lobby_waiting }, [](game_lobby &lobby, int user_id) {
            lobby.options = game_options{};
            lobby.broadcast_message(server_messages::lobby_game_options{ lobby.options });
        });

        lobby.add_command("quit", "QUIT_DESCRIPTION", {}, [](game_lobby &lobby, int user_id) {
            lobby.m_mgr->kick_client(lobby.find_user(user_id).session->client, "QUIT");
        });
    };

}