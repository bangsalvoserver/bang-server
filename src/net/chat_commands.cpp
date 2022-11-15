#include "chat_commands.h"

#include "manager.h"

#include "utils/enum_format.h"
#include "utils/static_map.h"

#include <charconv>

namespace banggame {

    static constexpr std::string_view HELP_DESCRIPTION = "print this message";
    static constexpr std::string_view USERS_DESCRIPTION = "print list of users in this lobby";
    static constexpr std::string_view KICK_DESCRIPTION = "[userid] : kick an user in this lobby";
    static constexpr std::string_view GET_OPTIONS_DESCRIPTION = "print game options";
    static constexpr std::string_view SET_OPTION_DESCRIPTION = "[name] [value] : set a game option";
    static constexpr std::string_view GIVE_CARD_DESCRIPTION = "[name] : give yourself a card (cheat)";

    const std::map<std::string, chat_command, std::less<>> chat_command::commands {
        { "help",           { proxy<&game_manager::command_print_help>,         HELP_DESCRIPTION }},
        { "users",          { proxy<&game_manager::command_print_users>,        USERS_DESCRIPTION }},
        { "kick",           { proxy<&game_manager::command_kick_user>,          KICK_DESCRIPTION, command_permissions::lobby_owner }},
        { "options",        { proxy<&game_manager::command_get_game_options>,   GET_OPTIONS_DESCRIPTION }},
        { "set-option",     { proxy<&game_manager::command_set_game_option>,    SET_OPTION_DESCRIPTION, command_permissions::lobby_owner | command_permissions::lobby_waiting }},
        { "give",           { proxy<&game_manager::command_give_card>,          GIVE_CARD_DESCRIPTION, command_permissions::game_cheat }}
    };

    std::string game_manager::command_print_help(user_ptr user) {
        for (const auto &[cmd_name, command] : chat_command::commands) {
            send_message<server_message_type::lobby_chat>(user->first, 0,
                fmt::format("{}{} : {}", chat_command::start_char, cmd_name, command.description()));
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

    template<size_t I>
    static std::string get_field_string(const game_options &options) {
        const auto field_data = reflector::get_field_data<I>(options);
        return fmt::format("{} = {}", field_data.name(), field_data.get());
    }

    template<size_t ... Is>
    static void print_game_options(game_manager &self, user_ptr user, const game_options &options, std::index_sequence<Is ...>) {
        (self.send_message<server_message_type::lobby_chat>(user->first, 0, get_field_string<Is>(options)), ...);
    }

    std::string game_manager::command_get_game_options(user_ptr user) {
        print_game_options(*this, user, (*user->second.in_lobby)->second.options, std::make_index_sequence<reflector::num_fields<game_options>>());
        return {};
    }

    template<size_t ... Is>
    constexpr auto gen_set_option_map(std::index_sequence<Is ...>) {
        using set_option_fn_ptr = bool (*)(game_options &options, std::string_view value_str);

        return util::static_map<std::string_view, set_option_fn_ptr>({
            { reflector::get_field_name<Is, game_options>(), [](game_options &options, std::string_view value_str) {
                auto field_data = reflector::get_field_data<Is>(options);
                auto &field = field_data.get();
                if (auto value = parse_string<std::remove_reference_t<decltype(field)>>(value_str)) {
                    field = *value;
                    return true;
                } else {
                    return false;
                }
            }} ... });
    }

    std::string game_manager::command_set_game_option(user_ptr user, std::string_view name, std::string_view value) {
        static constexpr auto set_option_map = gen_set_option_map(std::make_index_sequence<reflector::num_fields<game_options>>());
        
        if (auto it = set_option_map.find(name); it != set_option_map.end()) {
            auto &lobby = (*user->second.in_lobby)->second;

            if (it->second(lobby.options, value)) {
                broadcast_message_lobby<server_message_type::lobby_edited>(lobby, lobby);
                return {};
            } else {
                return "INVALID_OPTION_VALUE";
            }
        } else {
            return "INVALID_OPTION_NAME";
        }
    }

    std::string game_manager::command_give_card(user_ptr user, std::string_view name) {
        auto &lobby = (*user->second.in_lobby)->second;
        if (auto player_it = std::ranges::find(lobby.game.m_players, user->second.user_id, &player::user_id); player_it != lobby.game.m_players.end()) {
            if (auto card_it = std::ranges::find_if(lobby.game.m_deck, [name](std::string_view card_name) {
                return std::ranges::equal(name, card_name, {}, toupper, toupper);
            }, &card::name); card_it != lobby.game.m_deck.end()) {
                player_it->add_to_hand(*card_it);
                return {};
            } else {
                return "ERROR_CANNOT_GIVE_CARD";
            }
        } else {
            return "ERROR_USER_NOT_CONTROLLING_PLAYER";
        }
    }

}