#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "card_data.h"

namespace banggame {

    DEFINE_STRUCT(game_over_update,
        (player_role, winner_role)
    )

    DEFINE_STRUCT(card_backface,
        (int, id)
        (card_deck_type, deck)
    )

    DEFINE_STRUCT(add_cards_update,
        (std::vector<card_backface>, card_ids)
        (pocket_type, pocket)
        (serial::opt_player, player)
    )

    DEFINE_STRUCT(remove_cards_update,
        (std::vector<serial::card>, cards)
    )

    DEFINE_ENUM_FLAGS(show_card_flags,
        (pause_before_move)
        (short_pause)
        (instant)
        (shown)
        (hidden)
    )

    DEFINE_STRUCT(move_card_update,
        (serial::card, card)
        (serial::opt_player, player)
        (pocket_type, pocket)
        (show_card_flags, flags)
    )

    DEFINE_STRUCT(add_cubes_update,
        (int, num_cubes)
        (serial::opt_card, target_card)
    )

    DEFINE_STRUCT(move_cubes_update,
        (int, num_cubes)
        (serial::opt_card, origin_card)
        (serial::opt_card, target_card)
    )

    DEFINE_STRUCT(show_card_update,
        (serial::card, card)
        (card_data, info)
        (show_card_flags, flags)
    )

    DEFINE_STRUCT(hide_card_update,
        (serial::card, card)
        (show_card_flags, flags)
    )

    DEFINE_STRUCT(tap_card_update,
        (serial::card, card)
        (bool, inactive)
        (bool, instant)
    )

    DEFINE_STRUCT(player_add_update,
        (int, num_players)
    )

    DEFINE_STRUCT(player_user_update,
        (serial::player, player)
        (int, user_id)
    )

    DEFINE_STRUCT(player_remove_update,
        (serial::player, player)
        (bool, instant)
    )

    DEFINE_STRUCT(player_hp_update,
        (serial::player, player)
        (int, hp)
        (bool, instant)
    )

    DEFINE_STRUCT(player_gold_update,
        (serial::player, player)
        (int, gold)
    )

    DEFINE_STRUCT(player_show_role_update,
        (serial::player, player)
        (player_role, role)
        (bool, instant)
    )

    DEFINE_ENUM_FLAGS(game_flags,
        (game_over)
        (invert_rotation)
        (disable_equipping)
        (phase_one_draw_discard)
        (phase_one_override)
        (disable_player_distances)
    )

    DEFINE_ENUM_FLAGS(player_flags,
        (dead)
        (ghost)
        (temp_ghost)
        (targetable)
        (extra_turn)
        (treat_missed_as_bang)
        (role_revealed)
        (removed)
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
        (bool, character_choice, true)
        (bool, keep_last_card_shuffling)
        (bool, can_play_beer_in_duel, true)
        (bool, auto_discard_all)
        (int, scenario_deck_size, 12)
        (int, damage_timer_ms, 1500)
    )

    DEFINE_ENUM_TYPES(game_update_type,
        (game_over, game_over_update)
        (game_error, game_string)
        (game_log, game_string)
        (game_prompt, game_string)
        (add_cards, add_cards_update)
        (remove_cards, remove_cards_update)
        (move_card, move_card_update)
        (add_cubes, add_cubes_update)
        (move_cubes, move_cubes_update)
        (move_scenario_deck, serial::player)
        (deck_shuffled, pocket_type)
        (show_card, show_card_update)
        (hide_card, hide_card_update)
        (tap_card, tap_card_update)
        (flash_card, serial::card)
        (last_played_card, serial::opt_card)
        (player_add, player_add_update)
        (player_user, player_user_update)
        (player_remove, player_remove_update)
        (player_hp, player_hp_update)
        (player_gold, player_gold_update)
        (player_show_role, player_show_role_update)
        (player_status, player_status_update)
        (switch_turn, serial::player)
        (request_status, request_status_args)
        (game_flags, game_flags)
        (game_options, game_options)
        (status_clear)
        (confirm_play)
    )

    using game_update = enums::enum_variant<game_update_type>;
    #define UPD_TAG(name) enums::enum_tag_t<game_update_type::name>

    DEFINE_STRUCT(play_card_args,
        (serial::card, card)
        (std::vector<serial::card>, modifiers)
        (target_list, targets)
        (bool, is_response)
    )

    DEFINE_ENUM_TYPES(game_action_type,
        (pick_card, serial::card)
        (play_card, play_card_args)
        (prompt_respond, bool)
        (request_confirm)
    )

    using game_action = enums::enum_variant<game_action_type>;
}

#endif