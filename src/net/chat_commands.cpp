#include "chat_commands.h"

#include "manager.h"

#include "utils/enum_format.h"
#include "utils/static_map.h"

#include "cards/filter_enums.h"

#include <charconv>

namespace banggame {

    static constexpr std::string_view HELP_DESCRIPTION = "print this message";
    static constexpr std::string_view USERS_DESCRIPTION = "print list of users in this lobby";
    static constexpr std::string_view KICK_DESCRIPTION = "[userid] : kick an user in this lobby";
    static constexpr std::string_view GET_OPTIONS_DESCRIPTION = "print game options";
    static constexpr std::string_view SET_OPTION_DESCRIPTION = "[name] [value] : set a game option";
    static constexpr std::string_view GIVE_CARD_DESCRIPTION = "[name] : give yourself a card";
    static constexpr std::string_view SET_TEAM_DESCRIPTION = "[game_player / game_spectator] : set team";
    static constexpr std::string_view GET_RNG_SEED_DESCRIPTION = "print rng seed";
    static constexpr std::string_view REJOIN_DESCRIPTION = "[character] : rejoin disconnected player";

    const std::map<std::string, chat_command, std::less<>> chat_command::commands {
        { "help",           { proxy<&game_manager::command_print_help>,         HELP_DESCRIPTION }},
        { "users",          { proxy<&game_manager::command_print_users>,        USERS_DESCRIPTION }},
        { "kick",           { proxy<&game_manager::command_kick_user>,          KICK_DESCRIPTION, command_permissions::lobby_owner }},
        { "options",        { proxy<&game_manager::command_get_game_options>,   GET_OPTIONS_DESCRIPTION }},
        { "set-option",     { proxy<&game_manager::command_set_game_option>,    SET_OPTION_DESCRIPTION, command_permissions::lobby_owner | command_permissions::lobby_waiting }},
        { "give",           { proxy<&game_manager::command_give_card>,          GIVE_CARD_DESCRIPTION, command_permissions::game_cheat }},
        { "set-team",       { proxy<&game_manager::command_set_team>,           SET_TEAM_DESCRIPTION, command_permissions::lobby_waiting }},
        { "seed",           { proxy<&game_manager::command_get_rng_seed>,       GET_RNG_SEED_DESCRIPTION, command_permissions::lobby_finished }},
        { "rejoin",         { proxy<&game_manager::command_rejoin>,             REJOIN_DESCRIPTION, command_permissions::lobby_playing }}
    };

    std::string game_manager::command_print_help(user_ptr user) {
        for (const auto &[cmd_name, command] : chat_command::commands) {
            if (!bool(command.permissions() & command_permissions::game_cheat) || m_options.enable_cheats) {
                send_message<server_message_type::lobby_chat>(user->first, 0,
                    fmt::format("{}{} : {}", chat_command::start_char, cmd_name, command.description()));
            }
        }
        return {};
    }

    std::string game_manager::command_print_users(user_ptr user) {
        auto &lobby = *user->second.in_lobby;
        for (auto [team, lobby_user] : lobby.users) {
            send_message<server_message_type::lobby_chat>(user->first, 0,
                fmt::format("{} : {} ({})", lobby_user->second.user_id, lobby_user->second.name, enums::to_string(team)));
        }
        return {};
    }

    std::string game_manager::command_kick_user(user_ptr user, std::string_view userid_str) {
        int user_id;
        if (auto [end, ec] = std::from_chars(userid_str.data(), userid_str.data() + userid_str.size(), user_id); ec != std::errc{}) {
            return "INVALID_USERID_STRING";
        }

        auto &lobby = *user->second.in_lobby;
        auto kicked = std::ranges::find(lobby.users, user_id, [](const team_user_pair &pair) {
            return pair.second->second.user_id;
        });
        if (kicked == lobby.users.end()) {
            return "CANNOT_FIND_USERID";
        }
        kick_user_from_lobby(kicked->second);

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
        print_game_options(*this, user, user->second.in_lobby->options, std::make_index_sequence<reflector::num_fields<game_options>>());
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
            auto &lobby = *user->second.in_lobby;

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
        auto &lobby = *user->second.in_lobby;

        player *target = lobby.m_game->find_player_by_userid(user->second.user_id);
        if (!target) return "ERROR_USER_NOT_CONTROLLING_PLAYER";

        if (lobby.m_game->pending_requests() || lobby.m_game->pending_updates() || lobby.m_game->m_playing != target) {
            return "ERROR_PLAYER_NOT_IN_TURN";
        }

        auto card_it = std::ranges::find_if(lobby.m_game->m_context.cards, [&](const card &target_card) {
            if (std::ranges::equal(name, target_card.name, {}, toupper, toupper)) {
                switch (target_card.deck) {
                case card_deck_type::main_deck:
                    return target_card.pocket != pocket_type::player_hand || target_card.owner != target;
                case card_deck_type::character:
                    return target_card.pocket != pocket_type::player_character && target_card.pocket != pocket_type::player_backup;
                case card_deck_type::goldrush:
                    return target_card.pocket != pocket_type::shop_selection && target_card.pocket != pocket_type::hidden_deck
                        && (target_card.pocket != pocket_type::player_table || target_card.owner != target);
                case card_deck_type::highnoon:
                case card_deck_type::fistfulofcards:
                    return target->m_game->m_scenario_cards.empty() || &target_card != target->m_game->m_scenario_cards.back();
                case card_deck_type::wildwestshow:
                    return target->m_game->m_wws_scenario_cards.empty() || &target_card != target->m_game->m_wws_scenario_cards.back();
                }
            }
            return false;
        });
        if (card_it == lobby.m_game->m_context.cards.end()) return "ERROR_CANNOT_FIND_CARD";
        card *target_card = &*card_it;
        
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
            for (card *c : target->m_characters) {
                target->disable_equip(c);
            }

            card *old_character = target->first_character();
            int ncubes = old_character->num_cubes;

            target->m_game->move_cubes(old_character, nullptr, ncubes);
            target->m_game->add_update<game_update_type::remove_cards>(std::vector{not_null{old_character}});

            old_character->pocket = pocket_type::none;
            old_character->owner = nullptr;
            old_character->visibility = card_visibility::hidden;

            target->m_characters.clear();
            target->m_characters.push_back(target_card);

            target_card->pocket = pocket_type::player_character;
            target_card->owner = target;

            target->m_game->add_update<game_update_type::add_cards>(std::vector{card_backface{target_card}}, pocket_type::player_character, target);
            target->m_game->set_card_visibility(target_card, nullptr, card_visibility::shown, true);

            target->reset_max_hp();
            target->enable_equip(target_card);
            target->m_game->add_cubes(target_card, ncubes);
            break;
        }
        case card_deck_type::goldrush: {
            target->m_game->move_card(target->m_game->m_shop_selection.front(), pocket_type::shop_discard);
            target->m_game->move_card(target_card, pocket_type::shop_selection);
            break;
        }
        case card_deck_type::highnoon:
        case card_deck_type::fistfulofcards: {
            if (target_card->pocket == pocket_type::scenario_deck) {
                if (auto it = std::ranges::find(target->m_game->m_scenario_deck, target_card); it != target->m_game->m_scenario_deck.end()) {
                    target->m_game->m_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update<game_update_type::add_cards>(std::vector{card_backface{target_card}}, pocket_type::scenario_deck);
                }
                target->m_game->m_scenario_deck.push_back(target_card);
                target->m_game->set_card_visibility(target_card, nullptr, card_visibility::shown, true);
                target->m_game->add_update<game_update_type::move_card>(target_card, nullptr, pocket_type::scenario_deck, true);
            } else {
                target->m_game->move_card(target_card, pocket_type::scenario_deck);
            }
            break;
        }
        case card_deck_type::wildwestshow: {
            if (target_card->pocket == pocket_type::wws_scenario_deck) {
                if (auto it = std::ranges::find(target->m_game->m_wws_scenario_deck, target_card); it != target->m_game->m_wws_scenario_deck.end()) {
                    target->m_game->m_wws_scenario_deck.erase(it);
                } else {
                    target->m_game->add_update<game_update_type::add_cards>(std::vector{card_backface{target_card}}, pocket_type::wws_scenario_deck);
                }
                target->m_game->m_wws_scenario_deck.push_back(target_card);
                target->m_game->set_card_visibility(target_card, nullptr, card_visibility::shown, true);
                target->m_game->add_update<game_update_type::move_card>(target_card, nullptr, pocket_type::wws_scenario_deck, true);
            } else {
                target->m_game->move_card(target_card, pocket_type::wws_scenario_deck);
            }
            break;
        }
        }

        lobby.m_game->send_request_status_clear();
        lobby.m_game->send_request_status_ready();

        return {};
    }

    std::string game_manager::command_set_team(user_ptr user, std::string_view value) {
        if (auto team = enums::from_string<lobby_team>(value)) {
            std::ranges::find(user->second.in_lobby->users, user, &team_user_pair::second)->first = *team;
            return {};
        } else {
            return "ERROR_INVALID_TEAM";
        }
    }

    std::string game_manager::command_get_rng_seed(user_ptr user) {
        send_message<server_message_type::lobby_chat>(user->first, 0, std::to_string(user->second.in_lobby->m_game->rng_seed));
        return {};
    }

    std::string game_manager::command_rejoin(user_ptr user, std::string_view value) {
        auto &lobby = *user->second.in_lobby;

        auto check_disconnected = [&](player *p) {
            return !ranges::contains(lobby.users, p->user_id, [](const team_user_pair &pair) {
                return pair.second->second.user_id;
            });
        };

        if (value.empty()) {
            auto &lobby = *user->second.in_lobby;
            for (player *p : lobby.m_game->m_players | std::views::filter(check_disconnected)) {
                if (card *c = p->first_character()) {
                    send_message<server_message_type::lobby_chat>(user->first, 0, fmt::format("/rejoin {}", c->name));
                }
            }
            return {};
        }

        lobby_team &user_team = std::ranges::find(lobby.users, user, &team_user_pair::second)->first;
        if (user_team != lobby_team::game_spectator) {
            return "ERROR_USER_NOT_SPECTATOR";
        }

        auto it = std::ranges::find_if(lobby.m_game->m_players, [&](player *p) {
            if (card *c = p->first_character()) {
                return std::ranges::equal(value, c->name, {}, toupper, toupper);
            }
            return false;
        });
        if (it == lobby.m_game->m_players.end()) {
            return "ERROR_CANNOT_FIND_CHARACTER";
        }

        player *disconnected = *it;
        if (!check_disconnected(disconnected)) {
            return "ERROR_PLAYER_NOT_DISCONNECTED";
        }

        user_team = lobby_team::game_player;

        disconnected->user_id = user->second.user_id;
        lobby.m_game->add_update<game_update_type::player_add>(std::vector{player_user_pair{ disconnected }});
        
        for (const auto &msg : lobby.m_game->get_rejoin_updates(disconnected)) {
            send_message<server_message_type::game_update>(user->first, msg);
        }
        for (const auto &msg : lobby.m_game->get_game_log_updates(disconnected)) {
            send_message<server_message_type::game_update>(user->first, msg);
        }

        return {};
    }

}