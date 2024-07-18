#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "game_options.h"

#include "cards/card_data.h"

#include "utils/tagged_variant.h"

namespace banggame {

    struct card_backface {
        int id;
        card_deck_type deck;
    };

    card_backface to_card_backface(const card *origin_card);

    inline std::vector<card_backface> to_card_backface_vector(auto &&range) {
        return range | rv::transform(to_card_backface) | rn::to<std::vector>;
    }

    struct add_cards_update {
        std::vector<card_backface> card_ids;
        pocket_type pocket;
        serial::opt_player player;
    };

    struct remove_cards_update {
        serial::card_list cards;
    };

    struct move_card_update {
        serial::card card;
        serial::opt_player player;
        pocket_type pocket;
        animation_duration duration = durations.move_card;
        bool front = false;
    };

    struct add_cubes_update {
        int num_cubes;
        serial::opt_card target_card;
    };

    struct move_cubes_update {
        int num_cubes;
        serial::opt_card origin_card;
        serial::opt_card target_card;
        animation_duration duration = durations.move_cubes;
    };

    struct move_train_update {
        int position;
        animation_duration duration = durations.move_train;
    };

    struct deck_shuffled_update {
        pocket_type pocket;
        animation_duration duration = durations.deck_shuffle;
    };

    struct show_card_update {
        serial::card card;
        card_data info;
        animation_duration duration = durations.flip_card;
    };

    struct hide_card_update {
        serial::card card;
        animation_duration duration = durations.flip_card;
    };

    struct tap_card_update {
        serial::card card;
        bool inactive;
        animation_duration duration = durations.tap_card;
    };

    struct flash_card_update {
        serial::card card;
        animation_duration duration = durations.flash_card;
    };

    struct short_pause_update {
        serial::opt_card card;
        animation_duration duration = durations.short_pause;
    };

    struct player_user_pair {
        int player_id;
        int user_id;
    };

    player_user_pair to_player_user_pair(const player *p);

    inline std::vector<player_user_pair> to_player_user_pair_vector(auto &&range) {
        return range | rv::transform(to_player_user_pair) | rn::to<std::vector>;
    }

    struct player_add_update {
        std::vector<player_user_pair> players;
    };

    struct player_order_update {
        serial::player_list players;
        animation_duration duration = durations.move_player;
    };

    struct player_hp_update {
        serial::player player;
        int hp;
        animation_duration duration = durations.player_hp;
    };

    struct player_gold_update {
        serial::player player;
        int gold;
    };

    struct player_show_role_update {
        serial::player player;
        player_role role;
        animation_duration duration = durations.flip_card;
    };

    struct player_flags_update {
        serial::player player;
        player_flags flags;
    };

    using playable_cards_list = std::vector<serial::card_list>;

    struct player_distance_item {
        serial::player player;
        int distance;
    };

    struct player_distances {
        std::vector<player_distance_item> distances;
        int range_mod;
        int weapon_range;
    };

    struct timer_status_args {
        timer_id_t timer_id;
        game_duration duration;
    };

    struct request_status_args {
        serial::opt_card origin_card;
        serial::opt_player origin;
        serial::opt_player target;
        game_string status_text;
        playable_cards_list respond_cards;
        serial::card_list pick_cards;
        serial::card_list highlight_cards;
        serial::player_list target_set;
        player_distances distances;
        std::optional<timer_status_args> timer;
    };

    struct status_ready_args {
        playable_cards_list play_cards;
        player_distances distances;
    };

    using game_update = utils::tagged_variant<
        utils::tag<"game_error", game_string>,
        utils::tag<"game_log", game_string>,
        utils::tag<"game_prompt", game_string>,
        utils::tag<"add_cards", add_cards_update>,
        utils::tag<"remove_cards", remove_cards_update>,
        utils::tag<"move_card", move_card_update>,
        utils::tag<"add_cubes", add_cubes_update>,
        utils::tag<"move_cubes", move_cubes_update>,
        utils::tag<"move_train", move_train_update>,
        utils::tag<"deck_shuffled", deck_shuffled_update>,
        utils::tag<"show_card", show_card_update>,
        utils::tag<"hide_card", hide_card_update>,
        utils::tag<"tap_card", tap_card_update>,
        utils::tag<"flash_card", flash_card_update>,
        utils::tag<"short_pause", short_pause_update>,
        utils::tag<"player_add", player_add_update>,
        utils::tag<"player_order", player_order_update>,
        utils::tag<"player_hp", player_hp_update>,
        utils::tag<"player_gold", player_gold_update>,
        utils::tag<"player_show_role", player_show_role_update>,
        utils::tag<"player_flags", player_flags_update>,
        utils::tag<"switch_turn", serial::player>,
        utils::tag<"request_status", request_status_args>,
        utils::tag<"status_ready", status_ready_args>,
        utils::tag<"game_flags", game_flags>,
        utils::tag<"play_sound", std::string>,
        utils::tag<"status_clear">,
        utils::tag<"clear_logs">
    >;

    template<utils::fixed_string Name>
    concept game_update_type = utils::tag_for<utils::tag<Name>, game_update>;

    struct modifier_pair {
        serial::card card;
        target_list targets;
    };

    using modifier_list = std::vector<modifier_pair>;

    struct game_action {
        serial::card card;
        modifier_list modifiers;
        target_list targets;
        bool bypass_prompt;
        std::optional<timer_id_t> timer_id;
    };
}

#endif