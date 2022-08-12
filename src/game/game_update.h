#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "card_data.h"

namespace banggame {

    struct game_over_update {REFLECTABLE(
        (player_role) winner_role
    )};

    struct card_backface {REFLECTABLE(
        (int) id,
        (card_deck_type) deck
    )};

    struct add_cards_update {REFLECTABLE(
        (std::vector<card_backface>) card_ids,
        (pocket_type) pocket,
        (int) player_id
    )};

    struct remove_cards_update {REFLECTABLE(
        (std::vector<card_backface>) card_ids
    )};

    DEFINE_ENUM_FLAGS(show_card_flags,
        (pause_before_move)
        (short_pause)
        (instant)
        (shown)
        (hidden)
    )

    struct move_card_update {REFLECTABLE(
        (int) card_id,
        (int) player_id,
        (pocket_type) pocket,
        (show_card_flags) flags
    )};

    struct add_cubes_update {REFLECTABLE(
        (int) num_cubes,
        (int) target_card_id
    )};

    struct move_cubes_update {REFLECTABLE(
        (int) num_cubes,
        (int) origin_card_id,
        (int) target_card_id
    )};

    struct move_scenario_deck_args {REFLECTABLE(
        (int) player_id
    )};

    struct show_card_update {REFLECTABLE(
        (card_data) info,
        (show_card_flags) flags
    )};

    struct hide_card_update {REFLECTABLE(
        (int) card_id,
        (show_card_flags) flags
    )};

    struct tap_card_update {REFLECTABLE(
        (int) card_id,
        (bool) inactive,
        (bool) instant
    )};

    struct card_id_args {REFLECTABLE(
        (int) card_id
    )};

    struct player_user_update {REFLECTABLE(
        (int) player_id,
        (int) user_id
    )};

    struct player_remove_update {REFLECTABLE(
        (int) player_id
    )};

    struct player_hp_update {REFLECTABLE(
        (int) player_id,
        (int) hp,
        (bool) instant
    )};

    struct player_gold_update {REFLECTABLE(
        (int) player_id,
        (int) gold
    )};

    struct player_show_role_update {REFLECTABLE(
        (int) player_id,
        (player_role) role,
        (bool) instant
    )};

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
    )

    struct player_status_update {REFLECTABLE(
        (int) player_id,
        (player_flags) flags,
        (int) range_mod,
        (int) weapon_range,
        (int) distance_mod
    )};

    struct switch_turn_update {REFLECTABLE(
        (int) player_id
    )};

    struct picking_args {REFLECTABLE(
        (pocket_type) pocket,
        (int) player_id,
        (int) card_id
    )};

    struct request_status_args {REFLECTABLE(
        (int) origin_card_id,
        (int) origin_id,
        (int) target_id,
        (game_formatted_string) status_text,
        (effect_flags) flags,
        (std::vector<int>) respond_ids,
        (std::vector<picking_args>) pick_ids
    )};

    struct game_options {REFLECTABLE(
        (card_expansion_type) expansions,
        (bool) keep_last_card_shuffling,
        (int) scenario_deck_size
    )
        game_options() : scenario_deck_size(12) {}
    };

    DEFINE_ENUM_TYPES(game_update_type,
        (game_over, game_over_update)
        (game_error, game_formatted_string)
        (game_log, game_formatted_string)
        (game_prompt, game_formatted_string)
        (add_cards, add_cards_update)
        (remove_cards, remove_cards_update)
        (move_card, move_card_update)
        (add_cubes, add_cubes_update)
        (move_cubes, move_cubes_update)
        (move_scenario_deck, move_scenario_deck_args)
        (deck_shuffled, pocket_type)
        (show_card, show_card_update)
        (hide_card, hide_card_update)
        (tap_card, tap_card_update)
        (last_played_card, card_id_args)
        (player_add, player_user_update)
        (player_remove, player_remove_update)
        (player_hp, player_hp_update)
        (player_gold, player_gold_update)
        (player_show_role, player_show_role_update)
        (player_status, player_status_update)
        (switch_turn, switch_turn_update)
        (request_status, request_status_args)
        (game_flags, game_flags)
        (game_options, game_options)
        (status_clear)
        (confirm_play)
    )

    using game_update = enums::enum_variant<game_update_type>;
    #define UPD_TAG(name) enums::enum_tag_t<game_update_type::name>
}

#endif