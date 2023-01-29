#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "card_data.h"
#include "durations.h"

namespace banggame {

    DEFINE_STRUCT(card_backface,
        (int, id)
        (card_deck_type, deck),

        card_backface() = default;
        card_backface(card *c);
    )

    DEFINE_STRUCT(add_cards_update,
        (std::vector<card_backface>, card_ids)
        (pocket_type, pocket)
        (serial::opt_player, player)
    )

    DEFINE_STRUCT(remove_cards_update,
        (std::vector<serial::card>, cards)
    )

    DEFINE_STRUCT(move_card_update,
        (serial::card, card)
        (serial::opt_player, player)
        (pocket_type, pocket)
        (bool, instant),

        auto get_duration() const {
            return instant ? 0ms : durations::move_card;
        }
    )

    DEFINE_STRUCT(add_cubes_update,
        (int, num_cubes)
        (serial::opt_card, target_card)
    )

    DEFINE_STRUCT(move_cubes_update,
        (int, num_cubes)
        (serial::opt_card, origin_card)
        (serial::opt_card, target_card),
        
        auto get_duration() const {
            return num_cubes == 1 ? durations::move_cube : durations::move_cubes;
        }
    )

    DEFINE_STRUCT(move_scenario_deck_update,
        (serial::player, player)
        (pocket_type, pocket),

        auto get_duration() const {
            return durations::move_card;
        }
    )

    DEFINE_STRUCT(deck_shuffled_update,
        (pocket_type, pocket),

        auto get_duration() const {
            return durations::deck_shuffle;
        }
    )

    DEFINE_STRUCT(show_card_update,
        (serial::card, card)
        (card_data, info)
        (bool, instant),

        auto get_duration() const {
            return instant ? 0ms : durations::flip_card;
        }
    )

    DEFINE_STRUCT(hide_card_update,
        (serial::card, card)
        (bool, instant),
        bool skip_count = false;

        auto get_duration() const {
            return (instant || skip_count) ? 0ms : durations::flip_card;
        }
    )

    DEFINE_STRUCT(tap_card_update,
        (serial::card, card)
        (bool, inactive)
        (bool, instant),

        auto get_duration() const {
            return instant ? 0ms : durations::tap_card;
        }
    )

    DEFINE_STRUCT(flash_card_update,
        (serial::card, card),

        auto get_duration() const {
            return durations::flash_card;
        }
    )

    DEFINE_STRUCT(short_pause_update,
        (serial::opt_card, card),

        auto get_duration() const {
            return durations::short_pause;
        }
    )

    DEFINE_STRUCT(player_add_update,
        (int, num_players)
    )

    DEFINE_STRUCT(player_order_update,
        (std::vector<serial::player>, players)
        (bool, instant),

        auto get_duration() const {
            return instant ? 0ms : durations::move_player;
        }
    )

    DEFINE_STRUCT(player_user_update,
        (serial::player, player)
        (int, user_id)
    )

    DEFINE_STRUCT(player_hp_update,
        (serial::player, player)
        (int, hp)
        (bool, instant),

        auto get_duration() const {
            return instant ? 0ms : durations::player_hp;
        }
    )

    DEFINE_STRUCT(player_gold_update,
        (serial::player, player)
        (int, gold)
    )

    DEFINE_STRUCT(player_show_role_update,
        (serial::player, player)
        (player_role, role)
        (bool, instant),

        auto get_duration() const {
            return instant ? 0ms : durations::flip_card;
        }
    )

    DEFINE_ENUM_FLAGS(game_flags,
        (game_over)
        (invert_rotation)
        (disable_equipping)
        (phase_one_draw_discard)
        (phase_one_override)
        (disable_player_distances)
        (treat_any_as_bang)
        (hands_shown)
        (free_for_all)
    )

    DEFINE_ENUM_FLAGS(player_flags,
        (dead)
        (ghost)
        (temp_ghost)
        (extra_turn)
        (treat_missed_as_bang)
        (role_revealed)
        (skip_turn)
        (removed)
        (winner)
    )

    DEFINE_STRUCT(player_status_update,
        (serial::player, player)
        (player_flags, flags)
        (int, range_mod)
        (int, weapon_range)
        (int, distance_mod)
    )

    DEFINE_STRUCT(request_status_args,
        (serial::opt_card, origin_card)
        (serial::opt_player, origin)
        (serial::opt_player, target)
        (game_string, status_text)
        (effect_flags, flags)
        (std::vector<serial::card>, respond_cards)
        (std::vector<serial::card>, pick_cards)
        (std::vector<serial::card>, highlight_cards)
    )

    DEFINE_STRUCT(game_options,
        (card_expansion_type, expansions)
        (bool, enable_ghost_cards, false)
        (bool, character_choice, true)
        (bool, allow_beer_in_duel, true)
        (bool, quick_discard_all)
        (int, scenario_deck_size, 12)
        (int, num_bots, 0)
        (std::chrono::milliseconds, damage_timer, 1500)
        (std::chrono::milliseconds, escape_timer, 3000)
        (std::chrono::milliseconds, tumbleweed_timer, 3000)
    )

    DEFINE_ENUM_TYPES(game_update_type,
        (game_error, game_string)
        (game_log, game_string)
        (game_prompt, game_string)
        (add_cards, add_cards_update)
        (remove_cards, remove_cards_update)
        (move_card, move_card_update)
        (add_cubes, add_cubes_update)
        (move_cubes, move_cubes_update)
        (move_scenario_deck, move_scenario_deck_update)
        (deck_shuffled, deck_shuffled_update)
        (show_card, show_card_update)
        (hide_card, hide_card_update)
        (tap_card, tap_card_update)
        (flash_card, flash_card_update)
        (short_pause, short_pause_update)
        (player_add, player_add_update)
        (player_order, player_order_update)
        (player_user, player_user_update)
        (player_hp, player_hp_update)
        (player_gold, player_gold_update)
        (player_show_role, player_show_role_update)
        (player_status, player_status_update)
        (switch_turn, serial::player)
        (request_status, request_status_args)
        (game_flags, game_flags)
        (play_sound, std::string)
        (status_clear)
        (confirm_play)
    )

    using game_update = enums::enum_variant<game_update_type>;
    #define UPD_TAG(name) enums::enum_tag_t<game_update_type::name>

    DEFINE_STRUCT(modifier_pair,
        (serial::card, card)
        (target_list, targets)
    )

    DEFINE_STRUCT(play_card_args,
        (serial::card, card)
        (std::vector<modifier_pair>, modifiers)
        (target_list, targets)
        (bool, is_response)
    )

    DEFINE_ENUM_TYPES(game_action_type,
        (pick_card, serial::card)
        (play_card, play_card_args)
        (prompt_respond, bool)
    )

    using game_action = enums::enum_variant<game_action_type>;
}

#endif