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
        (serial::opt_player) player
    )};

    struct remove_cards_update {REFLECTABLE(
        (std::vector<serial::card>) cards
    )};

    DEFINE_ENUM_FLAGS(show_card_flags,
        (pause_before_move)
        (short_pause)
        (instant)
        (shown)
        (hidden)
    )

    struct move_card_update {REFLECTABLE(
        (serial::card) card,
        (serial::opt_player) player,
        (pocket_type) pocket,
        (show_card_flags) flags
    )};

    struct add_cubes_update {REFLECTABLE(
        (int) num_cubes,
        (serial::opt_card) target_card
    )};

    struct move_cubes_update {REFLECTABLE(
        (int) num_cubes,
        (serial::opt_card) origin_card,
        (serial::opt_card) target_card
    )};

    struct show_card_update {REFLECTABLE(
        (serial::card) card,
        (card_data) info,
        (show_card_flags) flags
    )};

    struct hide_card_update {REFLECTABLE(
        (serial::card) card,
        (show_card_flags) flags
    )};

    struct tap_card_update {REFLECTABLE(
        (serial::card) card,
        (bool) inactive,
        (bool) instant
    )};

    struct player_add_update {REFLECTABLE(
        (int) num_players
    )};

    struct player_user_update {REFLECTABLE(
        (serial::player) player,
        (int) user_id
    )};

    struct player_remove_update {REFLECTABLE(
        (serial::player) player,
        (bool) instant
    )};

    struct player_hp_update {REFLECTABLE(
        (serial::player) player,
        (int) hp,
        (bool) instant
    )};

    struct player_gold_update {REFLECTABLE(
        (serial::player) player,
        (int) gold
    )};

    struct player_show_role_update {REFLECTABLE(
        (serial::player) player,
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
        (removed)
    )

    struct player_status_update {REFLECTABLE(
        (serial::player) player,
        (player_flags) flags,
        (int) range_mod,
        (int) weapon_range,
        (int) distance_mod
    )};

    struct picking_args {REFLECTABLE(
        (pocket_type) pocket,
        (serial::opt_player) player,
        (serial::opt_card) card
    )
        bool operator == (const picking_args &) const = default;
    };

    struct request_status_args {REFLECTABLE(
        (serial::opt_card) origin_card,
        (serial::opt_player) origin,
        (serial::opt_player) target,
        (game_string) status_text,
        (effect_flags) flags,
        (std::vector<serial::card>) respond_cards,
        (std::vector<picking_args>) pick_cards,
        (std::vector<serial::card>) highlight_cards
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

    struct play_card_args {REFLECTABLE(
        (serial::card) card,
        (std::vector<serial::card>) modifiers,
        (target_list) targets
    )};

    DEFINE_ENUM_TYPES(game_action_type,
        (pick_card, picking_args)
        (play_card, play_card_args)
        (respond_card, play_card_args)
        (prompt_respond, bool)
        (request_confirm)
    )

    using game_action = enums::enum_variant<game_action_type>;
}

#endif