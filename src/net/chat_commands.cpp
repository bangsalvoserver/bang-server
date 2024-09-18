#include "chat_commands.h"

#include "manager.h"

#include "game/give_card.h"

namespace banggame {

    static constexpr std::string_view HELP_DESCRIPTION = "print this message";
    static constexpr std::string_view USERS_DESCRIPTION = "print list of users in this lobby";
    static constexpr std::string_view KICK_DESCRIPTION = "[user] : kick an user in this lobby";
    static constexpr std::string_view MUTE_DESCRIPTION = "[user] : mute an user in this lobby";
    static constexpr std::string_view UNMUTE_DESCRIPTION = "[user] : unmute an user in this lobby";
    static constexpr std::string_view GET_OPTIONS_DESCRIPTION = "print game options";
    static constexpr std::string_view SET_OPTION_DESCRIPTION = "[name] [value] : set a game option";
    static constexpr std::string_view RESET_OPTIONS_DESCRIPTION = "reset game options";
    static constexpr std::string_view GIVE_CARD_DESCRIPTION = "[name] : give yourself a card";
    static constexpr std::string_view GET_RNG_SEED_DESCRIPTION = "print rng seed (only during game over screen)";
    static constexpr std::string_view QUIT_DESCRIPTION = "disconnect from server";

    const string_command_map chat_command::commands {
        { "help",           { proxy<&game_manager::command_print_help>,         HELP_DESCRIPTION }},
        { "users",          { proxy<&game_manager::command_print_users>,        USERS_DESCRIPTION }},
        { "kick",           { proxy<&game_manager::command_kick_user>,          KICK_DESCRIPTION, command_permissions::lobby_owner }},
        { "mute",           { proxy<&game_manager::command_mute_user>,          MUTE_DESCRIPTION, command_permissions::lobby_owner }},
        { "unmute",         { proxy<&game_manager::command_unmute_user>,        UNMUTE_DESCRIPTION, command_permissions::lobby_owner }},
        { "options",        { proxy<&game_manager::command_get_game_options>,   GET_OPTIONS_DESCRIPTION }},
        { "set-option",     { proxy<&game_manager::command_set_game_option>,    SET_OPTION_DESCRIPTION, { command_permissions::lobby_owner, command_permissions::lobby_waiting } }},
        { "reset-options",  { proxy<&game_manager::command_reset_game_options>, RESET_OPTIONS_DESCRIPTION, { command_permissions::lobby_owner, command_permissions::lobby_waiting } }},
        { "give",           { proxy<&game_manager::command_give_card>,          GIVE_CARD_DESCRIPTION, command_permissions::game_cheat }},
        { "seed",           { proxy<&game_manager::command_get_rng_seed>,       GET_RNG_SEED_DESCRIPTION, command_permissions::lobby_finished }},
        { "quit",           { proxy<&game_manager::command_quit>,               QUIT_DESCRIPTION }},
    };

    void game_manager::command_print_help(game_user &user) {
        for (const auto &[cmd_name, command] : chat_command::commands) {
            if (!command.permissions().check(command_permissions::game_cheat) || m_options.enable_cheats) {
                send_message<"lobby_message">(user.client,
                    std::format("{}{} : {}", chat_command::start_char, cmd_name, command.description()));
            }
        }
    }

    void game_manager::command_print_users(game_user &user) {
        auto &lobby = *user.in_lobby;
        for (const lobby_user &lu : lobby.users) {
            send_message<"lobby_message">(user.client,
                std::format("{} : {} ({})", lu.user_id, lu.user->username, enums::to_string(lu.team)));
        }
    }

    void game_manager::command_kick_user(game_user &user, std::string_view name_or_id) {
        lobby_user &lu = user.in_lobby->find_user(name_or_id);
        kick_user_from_lobby(*lu.user);
    }

    void game_manager::command_mute_user(game_user &user, std::string_view name_or_id) {
        lobby_user &lu = user.in_lobby->find_user(name_or_id);
        lu.flags.add(lobby_user_flag::muted);
    }

    void game_manager::command_unmute_user(game_user &user, std::string_view name_or_id) {
        lobby_user &lu = user.in_lobby->find_user(name_or_id);
        lu.flags.remove(lobby_user_flag::muted);
    }
    
    void game_manager::command_get_game_options(game_user &user) {
        std::string options = user.in_lobby->options.to_string();
        
        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = options.find('\n', prev)) != std::string::npos) {
            send_message<"lobby_message">(user.client, options.substr(prev, pos - prev));
            prev = pos + 1;
        }
    }

    void game_manager::command_set_game_option(game_user &user, std::string_view key, std::string_view value) {
        try {
            lobby &lobby = *user.in_lobby;
            lobby.options.set_option(key, value);
            broadcast_message_lobby<"lobby_edited">(lobby, lobby);
        } catch (const std::exception &e) {
            throw lobby_error(e.what());
        }
    }

    void game_manager::command_reset_game_options(game_user &user) {
        auto &lobby = *user.in_lobby;
        lobby.options = game_options::default_game_options;
        broadcast_message_lobby<"lobby_edited">(lobby, lobby);
    }

    void game_manager::command_give_card(game_user &user, std::string_view card_name) {
        auto &lobby = *user.in_lobby;
        lobby_user &lu = lobby.find_user(user);

        player_ptr target = lobby.m_game->find_player_by_userid(lu.user_id);
        if (!target) {
            throw lobby_error("ERROR_USER_NOT_CONTROLLING_PLAYER");
        }

        if (lobby.m_game->pending_requests() || lobby.m_game->is_waiting() || lobby.m_game->m_playing != target) {
            throw lobby_error("ERROR_PLAYER_NOT_IN_TURN");
        }

        if (!give_card(lobby.m_game.get(), target, card_name)) {
            throw lobby_error("ERROR_CANNOT_FIND_CARD");
        }
    }

    void game_manager::command_get_rng_seed(game_user &user) {
        send_message<"lobby_message">(user.client, std::to_string(user.in_lobby->m_game->rng_seed));
    }

    void game_manager::command_quit(game_user &user) {
        kick_client(user.client, "QUIT");
    }

}