#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "cards/card_data.h"
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
        (serial::card_list, cards)
    )

    DEFINE_STRUCT(move_card_update,
        (serial::card, card)
        (serial::opt_player, player)
        (pocket_type, pocket)
        (game_duration, duration),

        move_card_update() = default;
        move_card_update(serial::card card, serial::opt_player player, pocket_type pocket, bool instant = false)
            : card{card}, player{player}, pocket{pocket}
            , duration{instant ? 0ms : durations.move_card} {}
    )

    DEFINE_STRUCT(add_cubes_update,
        (int, num_cubes)
        (serial::opt_card, target_card)
    )

    DEFINE_STRUCT(move_cubes_update,
        (int, num_cubes)
        (serial::opt_card, origin_card)
        (serial::opt_card, target_card)
        (game_duration, duration),

        move_cubes_update() = default;
        move_cubes_update(int num_cubes, serial::opt_card origin_card, serial::opt_card target_card, bool instant = false)
            : num_cubes{num_cubes}, origin_card{origin_card}, target_card{target_card}
            , duration{instant ? 0ms : num_cubes == 1 ? durations.move_cube : durations.move_cubes} {}
    )

    DEFINE_STRUCT(move_train_update,
        (int, position)
        (game_duration, duration),

        move_train_update() = default;
        move_train_update(int position, bool instant = false)
            : position{position}
            , duration{instant ? 0ms : durations.move_train} {}
    )

    DEFINE_STRUCT(deck_shuffled_update,
        (pocket_type, pocket)
        (game_duration, duration),

        deck_shuffled_update() = default;
        deck_shuffled_update(pocket_type pocket, game_duration duration = durations.deck_shuffle)
            : pocket{pocket}, duration{duration} {}
    )

    DEFINE_STRUCT(show_card_update,
        (serial::card, card)
        (card_data, info)
        (game_duration, duration),

        show_card_update() = default;
        show_card_update(serial::card card, const card_data &info, bool instant = false)
            : card{card}, info{info}
            , duration{instant ? 0ms : durations.flip_card} {}
    )

    DEFINE_STRUCT(hide_card_update,
        (serial::card, card)
        (game_duration, duration),

        hide_card_update() = default;
        hide_card_update(serial::card card, bool instant = false)
            : card{card}
            , duration{instant ? 0ms : durations.flip_card} {}
    )

    DEFINE_STRUCT(tap_card_update,
        (serial::card, card)
        (bool, inactive)
        (game_duration, duration),

        tap_card_update() = default;
        tap_card_update(serial::card card, bool inactive, bool instant = false)
            : card{card}, inactive{inactive}
            , duration{instant ? 0ms : durations.tap_card} {}
    )

    DEFINE_STRUCT(flash_card_update,
        (serial::card, card)
        (game_duration, duration),

        flash_card_update() = default;
        flash_card_update(serial::card card, game_duration duration = durations.flash_card)
            : card{card}, duration{duration} {}
    )

    DEFINE_STRUCT(short_pause_update,
        (serial::opt_card, card)
        (game_duration, duration),

        short_pause_update() = default;
        short_pause_update(serial::opt_card card, game_duration duration = durations.short_pause)
            : card{card}, duration{duration} {}
    )

    DEFINE_STRUCT(player_user_pair,
        (int, player_id)
        (int, user_id),

        player_user_pair() = default;
        player_user_pair(player *p);
    )

    DEFINE_STRUCT(player_add_update,
        (std::vector<player_user_pair>, players)
    )

    DEFINE_STRUCT(player_order_update,
        (serial::player_list, players)
        (game_duration, duration),

        player_order_update() = default;
        player_order_update(const serial::player_list &players, bool instant = false)
            : players{players}
            , duration{instant ? 0ms : durations.move_player} {}
    )

    DEFINE_STRUCT(player_hp_update,
        (serial::player, player)
        (int, hp)
        (game_duration, duration),

        player_hp_update() = default;
        player_hp_update(serial::player player, int hp, bool instant = false)
            : player{player}, hp{hp}
            , duration{instant ? 0ms : durations.player_hp} {}
    )

    DEFINE_STRUCT(player_gold_update,
        (serial::player, player)
        (int, gold)
    )

    DEFINE_STRUCT(player_show_role_update,
        (serial::player, player)
        (player_role, role)
        (game_duration, duration),

        player_show_role_update() = default;
        player_show_role_update(serial::player player, player_role role, bool instant = false)
            : player{player}, role{role}
            , duration{instant ? 0ms : durations.flip_card} {}
    )

    DEFINE_STRUCT(player_flags_update,
        (serial::player, player)
        (player_flags, flags)
    )

    DEFINE_STRUCT(card_modifier_node,
        (serial::card, card)
        (std::vector<card_modifier_node>, branches)
    )

    using card_modifier_tree = std::vector<card_modifier_node>;

    DEFINE_STRUCT(player_distance_item,
        (serial::player, player)
        (int, distance)
    )

    DEFINE_STRUCT(player_distances,
        (std::vector<player_distance_item>, distances)
        (int, range_mod)
        (int, weapon_range)
    )

    DEFINE_STRUCT(timer_status_args,
        (timer_id_t, timer_id)
        (game_duration, duration)
    )

    DEFINE_STRUCT(request_status_args,
        (serial::opt_card, origin_card)
        (serial::opt_player, origin)
        (serial::opt_player, target)
        (game_string, status_text)
        (bool, auto_select)
        (card_modifier_tree, respond_cards)
        (serial::card_list, pick_cards)
        (serial::card_list, highlight_cards)
        (target_list, target_set)
        (player_distances, distances)
        (std::optional<timer_status_args>, timer)
    )

    DEFINE_STRUCT(status_ready_args,
        (card_modifier_tree, play_cards)
        (player_distances, distances)
    )

    DEFINE_STRUCT(game_options,
        (expansion_type, expansions)
        (bool, enable_ghost_cards, false)
        (bool, character_choice, true)
        (bool, allow_beer_in_duel, true)
        (bool, quick_discard_all)
        (int, scenario_deck_size, 12)
        (int, num_bots, 0)
        (game_duration, damage_timer, 1500)
        (game_duration, escape_timer, 3000)
        (game_duration, bot_play_timer, 500)
        (game_duration, tumbleweed_timer, 3000)
        (unsigned int, game_seed)
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
        (move_train, move_train_update)
        (deck_shuffled, deck_shuffled_update)
        (show_card, show_card_update)
        (hide_card, hide_card_update)
        (tap_card, tap_card_update)
        (flash_card, flash_card_update)
        (short_pause, short_pause_update)
        (player_add, player_add_update)
        (player_order, player_order_update)
        (player_hp, player_hp_update)
        (player_gold, player_gold_update)
        (player_show_role, player_show_role_update)
        (player_flags, player_flags_update)
        (switch_turn, serial::player)
        (request_status, request_status_args)
        (status_ready, status_ready_args)
        (game_flags, game_flags)
        (play_sound, std::string)
        (status_clear)
    )

    using game_update = enums::enum_variant<game_update_type>;
    #define UPD_TAG(name) enums::enum_tag_t<game_update_type::name>

    DEFINE_STRUCT(modifier_pair,
        (serial::card, card)
        (target_list, targets)
    )

    using modifier_list = std::vector<modifier_pair>;

    DEFINE_STRUCT(pick_card_args,
        (serial::card, card)
        (bool, bypass_prompt)
        (std::optional<timer_id_t>, timer_id)
    )

    DEFINE_STRUCT(play_card_args,
        (serial::card, card)
        (modifier_list, modifiers)
        (target_list, targets)
        (bool, bypass_prompt)
        (std::optional<timer_id_t>, timer_id)
    )

    DEFINE_ENUM_TYPES(game_action_type,
        (pick_card, pick_card_args)
        (play_card, play_card_args)
    )

    using game_action = enums::enum_variant<game_action_type>;
}

#endif