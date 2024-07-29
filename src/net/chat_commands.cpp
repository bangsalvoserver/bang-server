#include "chat_commands.h"

#include "manager.h"

#include "utils/parse_string.h"
#include "utils/static_map.h"

#include "cards/filter_enums.h"

#include <charconv>
#include <reflect>

namespace banggame {

    static constexpr std::string_view HELP_DESCRIPTION = "print this message";
    static constexpr std::string_view USERS_DESCRIPTION = "print list of users in this lobby";
    static constexpr std::string_view KICK_DESCRIPTION = "[userid] : kick an user in this lobby";
    static constexpr std::string_view GET_OPTIONS_DESCRIPTION = "print game options";
    static constexpr std::string_view SET_OPTION_DESCRIPTION = "[name] [value] : set a game option";
    static constexpr std::string_view RESET_OPTIONS_DESCRIPTION = "reset game options";
    static constexpr std::string_view GIVE_CARD_DESCRIPTION = "[name] : give yourself a card";
    static constexpr std::string_view SPECTATE_DESCRIPTION = "spectate this game";
    static constexpr std::string_view GET_RNG_SEED_DESCRIPTION = "print rng seed";
    static constexpr std::string_view QUIT_DESCRIPTION = "disconnect from server";

    const string_command_map chat_command::commands {
        { "help",           { proxy<&game_manager::command_print_help>,         HELP_DESCRIPTION }},
        { "users",          { proxy<&game_manager::command_print_users>,        USERS_DESCRIPTION }},
        { "kick",           { proxy<&game_manager::command_kick_user>,          KICK_DESCRIPTION, command_permissions::lobby_owner }},
        { "options",        { proxy<&game_manager::command_get_game_options>,   GET_OPTIONS_DESCRIPTION }},
        { "set-option",     { proxy<&game_manager::command_set_game_option>,    SET_OPTION_DESCRIPTION, { command_permissions::lobby_owner, command_permissions::lobby_waiting } }},
        { "reset-options",  { proxy<&game_manager::command_reset_game_options>, RESET_OPTIONS_DESCRIPTION, { command_permissions::lobby_owner, command_permissions::lobby_waiting } }},
        { "give",           { proxy<&game_manager::command_give_card>,          GIVE_CARD_DESCRIPTION, command_permissions::game_cheat }},
        { "spectate",       { proxy<&game_manager::command_spectate>,           SPECTATE_DESCRIPTION, command_permissions::lobby_waiting }},
        { "seed",           { proxy<&game_manager::command_get_rng_seed>,       GET_RNG_SEED_DESCRIPTION, command_permissions::lobby_finished }},
        { "quit",           { proxy<&game_manager::command_quit>,               QUIT_DESCRIPTION }},
    };

    void game_manager::command_print_help(game_user &user) {
        for (const auto &[cmd_name, command] : chat_command::commands) {
            if (!command.permissions().check(command_permissions::game_cheat) || m_options.enable_cheats) {
                send_message<"lobby_chat">(user.client, 0,
                    std::format("{}{} : {}", chat_command::start_char, cmd_name, command.description()));
            }
        }
    }

    void game_manager::command_print_users(game_user &user) {
        auto &lobby = *user.in_lobby;
        for (auto [team, user_id, u] : lobby.users) {
            send_message<"lobby_chat">(user.client, 0,
                std::format("{} : {} ({})", user_id, u->name, enums::to_string(team)));
        }
    }

    void game_manager::command_kick_user(game_user &user, std::string_view user_id_str) {
        int user_id;
        if (auto [end, ec] = std::from_chars(user_id_str.data(), user_id_str.data() + user_id_str.size(), user_id); ec != std::errc{}) {
            throw lobby_error("INVALID_USERID_STRING");
        }

        auto &lobby = *user.in_lobby;
        auto kicked = rn::find(lobby.users, user_id, &lobby_user::user_id);
        if (kicked == lobby.users.end()) {
            throw lobby_error("CANNOT_FIND_USERID");
        }
        kick_user_from_lobby(*(kicked->user));
    }

    void game_manager::command_get_game_options(game_user &user) {
        const game_options &options = user.in_lobby->options;
        reflect::for_each<game_options>([&](auto I) {
            send_message<"lobby_chat">(user.client, 0,
                std::format("{} = {}", reflect::member_name<I>(options), reflect::get<I>(options))
            );
        });
    }

    template<size_t ... Is>
    constexpr auto gen_set_option_map(std::index_sequence<Is ...>) {
        using set_option_fn_ptr = bool (*)(game_options &options, std::string_view value_str);

        return utils::static_map<std::string_view, set_option_fn_ptr>({
            { reflect::member_name<Is, game_options>(), [](game_options &options, std::string_view value_str) {
                auto &field = reflect::get<Is>(options);
                if (auto value = parse_string<std::remove_reference_t<decltype(field)>>(value_str)) {
                    field = *value;
                    return true;
                } else {
                    return false;
                }
            }} ... });
    }

    void game_manager::command_set_game_option(game_user &user, std::string_view name, std::string_view value) {
        static constexpr auto set_option_map = gen_set_option_map(std::make_index_sequence<reflect::size<game_options>()>());
        
        if (auto it = set_option_map.find(name); it != set_option_map.end()) {
            auto &lobby = *user.in_lobby;

            if (it->second(lobby.options, value)) {
                broadcast_message_lobby<"lobby_edited">(lobby, lobby);
            } else {
                throw lobby_error("INVALID_OPTION_VALUE");
            }
        } else {
            throw lobby_error("INVALID_OPTION_NAME");
        }
    }

    void game_manager::command_reset_game_options(game_user &user) {
        auto &lobby = *user.in_lobby;
        lobby.options = banggame::default_game_options;
        broadcast_message_lobby<"lobby_edited">(lobby, lobby);
    }

    void game_manager::command_give_card(game_user &user, std::string_view name) {
        auto &lobby = *user.in_lobby;

        player_ptr target = lobby.m_game->find_player_by_userid(lobby.get_user_id(user));
        if (!target) {
            throw lobby_error("ERROR_USER_NOT_CONTROLLING_PLAYER");
        }

        if (lobby.m_game->pending_requests() || lobby.m_game->is_waiting() || lobby.m_game->m_playing != target) {
            throw lobby_error("ERROR_PLAYER_NOT_IN_TURN");
        }

        auto all_cards = lobby.m_game->get_all_cards();
        auto card_it = rn::find_if(all_cards, [&](const_card_ptr target_card) {
            if (rn::equal(name, target_card->name, {}, toupper, toupper)) {
                switch (target_card->deck) {
                case card_deck_type::train:
                case card_deck_type::main_deck:
                    return target_card->pocket != pocket_type::player_hand || target_card->owner != target;
                case card_deck_type::character:
                    return target_card->pocket != pocket_type::player_character && target_card->pocket != pocket_type::player_backup;
                case card_deck_type::goldrush:
                    return target_card->pocket != pocket_type::shop_selection && target_card->pocket != pocket_type::hidden_deck
                        && (target_card->pocket != pocket_type::player_table || target_card->owner != target);
                case card_deck_type::highnoon:
                case card_deck_type::fistfulofcards:
                    return target->m_game->m_scenario_cards.empty() || target_card != target->m_game->m_scenario_cards.back();
                case card_deck_type::wildwestshow:
                    return target->m_game->m_wws_scenario_cards.empty() || target_card != target->m_game->m_wws_scenario_cards.back();
                }
            }
            return false;
        });
        if (card_it == all_cards.end()) {
            throw lobby_error("ERROR_CANNOT_FIND_CARD");
        }
        card_ptr target_card = *card_it;
        
        lobby.m_game->send_request_status_clear();
        
        switch (target_card->deck) {
        case card_deck_type::main_deck: {
            if (target_card->owner) {
                target->steal_card(target_card);
            } else {
                target->add_to_hand(target_card);
            }
            break;
        }
        case card_deck_type::character: {
            target->remove_extra_characters();
            for (card_ptr c : target->m_characters) {
                target->disable_equip(c);
            }

            card_ptr old_character = target->first_character();
            int ncubes = old_character->num_cubes;

            old_character->move_cubes(nullptr, ncubes);
            target->m_game->add_update<"remove_cards">(std::vector{old_character});

            old_character->pocket = pocket_type::none;
            old_character->owner = nullptr;
            old_character->visibility = card_visibility::hidden;

            target->m_characters.clear();
            target->m_characters.push_back(target_card);

            target_card->pocket = pocket_type::player_character;
            target_card->owner = target;

            target->m_game->add_update<"add_cards">(target_card, pocket_type::player_character, target);
            target_card->set_visibility(card_visibility::shown, nullptr, true);

            target->reset_max_hp();
            target->enable_equip(target_card);
            target_card->add_cubes(ncubes);
            break;
        }
        case card_deck_type::goldrush: {
            target->m_game->m_shop_selection.front()->move_to(pocket_type::shop_discard);
            target_card->move_to(pocket_type::shop_selection);
            break;
        }
        case card_deck_type::highnoon:
        case card_deck_type::fistfulofcards: {
            if (target_card->pocket == pocket_type::scenario_deck) {
                if (auto it = rn::find(target->m_game->m_scenario_deck, target_card); it != target->m_game->m_scenario_deck.end()) {
                    target->m_game->m_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update<"add_cards">(target_card, pocket_type::scenario_deck);
                }
                target->m_game->m_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                target->m_game->add_update<"move_card">(target_card, nullptr, pocket_type::scenario_deck, 0ms);
            } else {
                target_card->move_to(pocket_type::scenario_deck);
            }
            break;
        }
        case card_deck_type::wildwestshow: {
            if (target_card->pocket == pocket_type::wws_scenario_deck) {
                if (auto it = rn::find(target->m_game->m_wws_scenario_deck, target_card); it != target->m_game->m_wws_scenario_deck.end()) {
                    target->m_game->m_wws_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update<"add_cards">(target_card, pocket_type::wws_scenario_deck);
                }
                target->m_game->m_wws_scenario_deck.push_back(target_card);
                target_card->set_visibility(card_visibility::shown, nullptr, true);
                target->m_game->add_update<"move_card">(target_card, nullptr, pocket_type::wws_scenario_deck, 0ms);
            } else {
                target_card->move_to(pocket_type::wws_scenario_deck);
            }
            break;
        }
        case card_deck_type::train: {
            if (target_card->owner) {
                target->steal_card(target_card);
            } else {
                bool from_train = target_card->pocket == pocket_type::train;
                target->equip_card(target_card);
                if (card_ptr drawn_card = target->m_game->top_train_card(); from_train && drawn_card) {
                    drawn_card->move_to(pocket_type::train);
                }
            }
            break;
        }
        }

        lobby.m_game->commit_updates();
    }

    void game_manager::command_spectate(game_user &user) {
        set_user_team(user, lobby_team::game_spectator);
    }

    void game_manager::command_get_rng_seed(game_user &user) {
        send_message<"lobby_chat">(user.client, 0, std::to_string(user.in_lobby->m_game->rng_seed));
    }

    void game_manager::command_quit(game_user &user) {
        kick_client(user.client, "QUIT");
    }

}