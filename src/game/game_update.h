#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "cards/card_data.h"
#include "durations.h"

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
        game_duration duration = durations.move_card;
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
        game_duration duration = durations.move_cubes;
    };

    struct move_train_update {
        int position;
        game_duration duration = durations.move_train;
    };

    struct deck_shuffled_update {
        pocket_type pocket;
        game_duration duration = durations.deck_shuffle;
    };

    struct show_card_update {
        serial::card card;
        card_data info;
        game_duration duration = durations.flip_card;
    };

    struct hide_card_update {
        serial::card card;
        game_duration duration = durations.flip_card;
    };

    struct tap_card_update {
        serial::card card;
        bool inactive;
        game_duration duration = durations.tap_card;
    };

    struct flash_card_update {
        serial::card card;
        game_duration duration = durations.flash_card;
    };

    struct short_pause_update {
        serial::opt_card card;
        game_duration duration = durations.short_pause;
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
        game_duration duration = durations.move_player;
    };

    struct player_hp_update {
        serial::player player;
        int hp;
        game_duration duration = durations.player_hp;
    };

    struct player_gold_update {
        serial::player player;
        int gold;
    };

    struct player_show_role_update {
        serial::player player;
        player_role role;
        game_duration duration = durations.flip_card;
    };

    struct player_flags_update {
        serial::player player;
        player_flags flags;
    };

    struct card_modifier_node {
        serial::card card;
        std::vector<card_modifier_node> branches;
    };

    using card_modifier_tree = std::vector<card_modifier_node>;

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
        card_modifier_tree respond_cards;
        serial::card_list pick_cards;
        serial::card_list highlight_cards;
        serial::player_list target_set;
        player_distances distances;
        std::optional<timer_status_args> timer;
    };

    struct status_ready_args {
        card_modifier_tree play_cards;
        player_distances distances;
    };

    struct game_options {
        enums::bitset<expansion_type> expansions;
        bool enable_ghost_cards = false;
        bool character_choice = true;
        bool quick_discard_all = true;
        int scenario_deck_size = 12;
        int num_bots = 0;
        game_duration damage_timer = 1500ms;
        game_duration escape_timer = 3000ms;
        game_duration bot_play_timer = 500ms;
        game_duration tumbleweed_timer = 3000ms;
        unsigned int game_seed = 0;
    };

    enum class game_update_type {
        game_error,
        game_log,
        game_prompt,
        add_cards,
        remove_cards,
        move_card,
        add_cubes,
        move_cubes,
        move_train,
        deck_shuffled,
        show_card,
        hide_card,
        tap_card,
        flash_card,
        short_pause,
        player_add,
        player_order,
        player_hp,
        player_gold,
        player_show_role,
        player_flags,
        switch_turn,
        request_status,
        status_ready,
        game_flags,
        play_sound,
        status_clear,
        clear_logs,
    };

    using game_update = enums::enum_variant<game_update_type,
        enums::type_assoc<game_update_type::game_error, game_string>,
        enums::type_assoc<game_update_type::game_log, game_string>,
        enums::type_assoc<game_update_type::game_prompt, game_string>,
        enums::type_assoc<game_update_type::add_cards, add_cards_update>,
        enums::type_assoc<game_update_type::remove_cards, remove_cards_update>,
        enums::type_assoc<game_update_type::move_card, move_card_update>,
        enums::type_assoc<game_update_type::add_cubes, add_cubes_update>,
        enums::type_assoc<game_update_type::move_cubes, move_cubes_update>,
        enums::type_assoc<game_update_type::move_train, move_train_update>,
        enums::type_assoc<game_update_type::deck_shuffled, deck_shuffled_update>,
        enums::type_assoc<game_update_type::show_card, show_card_update>,
        enums::type_assoc<game_update_type::hide_card, hide_card_update>,
        enums::type_assoc<game_update_type::tap_card, tap_card_update>,
        enums::type_assoc<game_update_type::flash_card, flash_card_update>,
        enums::type_assoc<game_update_type::short_pause, short_pause_update>,
        enums::type_assoc<game_update_type::player_add, player_add_update>,
        enums::type_assoc<game_update_type::player_order, player_order_update>,
        enums::type_assoc<game_update_type::player_hp, player_hp_update>,
        enums::type_assoc<game_update_type::player_gold, player_gold_update>,
        enums::type_assoc<game_update_type::player_show_role, player_show_role_update>,
        enums::type_assoc<game_update_type::player_flags, player_flags_update>,
        enums::type_assoc<game_update_type::switch_turn, serial::player>,
        enums::type_assoc<game_update_type::request_status, request_status_args>,
        enums::type_assoc<game_update_type::status_ready, status_ready_args>,
        enums::type_assoc<game_update_type::game_flags, game_flags>,
        enums::type_assoc<game_update_type::play_sound, std::string>
    >;

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