#include "chat_commands.h"

#include "manager.h"

#include <charconv>

namespace banggame {

    static constexpr std::string_view HELP_DESCRIPTION = "print this message";
    static constexpr std::string_view USERS_DESCRIPTION = "print list of users in this lobby";
    static constexpr std::string_view KICK_DESCRIPTION = "kick an user in this lobby";

    const std::map<std::string, chat_command, std::less<>> chat_command::commands {
        { "help",       { proxy<&game_manager::command_print_help>,     HELP_DESCRIPTION }},
        { "users",      { proxy<&game_manager::command_print_users>,    USERS_DESCRIPTION }},
        { "kick",       { proxy<&game_manager::command_kick_user>,      KICK_DESCRIPTION, command_permissions::lobby_owner }}
    };

    std::string game_manager::command_print_help(user_ptr user) {
        for (const auto &[cmd_name, command] : chat_command::commands) {
            if (command.nargs() == 0) {
                send_message<server_message_type::lobby_chat>(user->first, 0,
                    fmt::format("{}{} : {}", chat_command::start_char, cmd_name, command.description()));
            } else if (command.nargs() == 1) {
                send_message<server_message_type::lobby_chat>(user->first, 0,
                    fmt::format("{}{} (1 arg) : {}", chat_command::start_char, cmd_name, command.description()));
            } else {
                send_message<server_message_type::lobby_chat>(user->first, 0,
                    fmt::format("{}{} ({} args) : {}", chat_command::start_char, cmd_name, command.nargs(), command.description()));
            }
        }
        return {};
    }

    std::string game_manager::command_print_users(user_ptr user) {
        auto &lobby = (*user->second.in_lobby)->second;
        for (user_ptr lobby_user : lobby.users) {
            send_message<server_message_type::lobby_chat>(user->first, 0,
                fmt::format("{} : {}", lobby_user->second.user_id, lobby_user->second.name));
        }
        return {};
    }

    std::string game_manager::command_kick_user(user_ptr user, std::string_view userid_str) {
        int user_id;
        if (auto [end, ec] = std::from_chars(userid_str.data(), userid_str.data() + userid_str.size(), user_id); ec != std::errc{}) {
            return "INVALID_USERID_STRING";
        }

        auto &lobby = (*user->second.in_lobby)->second;
        auto kicked = std::ranges::find(lobby.users, user_id, [](user_ptr lobby_user) {
            return lobby_user->second.user_id;
        });
        if (kicked == lobby.users.end()) {
            return "CANNOT_FIND_USERID";
        }
        kick_user_from_lobby(*kicked);

        return {};
    }

}