#ifndef __GAME_UPDATE_H__
#define __GAME_UPDATE_H__

#include "durations.h"
#include "cards/card_data.h"

namespace banggame {

    struct card_backface_list {
        card_list cards;

        card_backface_list() = default;
        card_backface_list(card_list cards): cards{std::move(cards)} {}
        card_backface_list(card_ptr card): cards{card} {}
    };

    enum class pocket_position {
        begin,
        random,
        end,
    };

    struct player_user_list {
        player_list players;

        player_user_list() = default;
        player_user_list(player_list players): players{std::move(players)} {}
        player_user_list(player_ptr player): players{player} {}
    };

    struct playable_card_info {
        card_ptr card;
        card_list modifiers;
        effect_context_base context;
    };

    using playable_cards_list = std::vector<playable_card_info>;
    
    using timer_id_t = size_t;

    namespace token_positions {
        struct table {};

        struct card {
            struct transparent{};
            card_ptr card;
        };

        struct player {
            struct transparent{};
            player_ptr player;
        };
    }

    using token_position = std::variant<
        token_positions::table,
        token_positions::card,
        token_positions::player
    >;

    namespace game_updates {

        struct preload_assets {
            std::vector<std::string> images;
            std::vector<sound_id> sounds;
        };

        struct game_error {
            struct transparent{};
            game_string message;
        };

        struct game_log {
            struct transparent{};
            game_string message;
        };

        struct game_prompt {
            struct transparent{};
            game_string message;
        };

        struct add_cards {
            card_backface_list card_ids;
            pocket_type pocket;
            nullable_player player;
        };

        struct remove_cards {
            card_list cards;
        };

        struct move_card {
            card_ptr card;
            nullable_player player;
            pocket_type pocket;
            animation_duration duration = durations.move_card;
            pocket_position position = pocket_position::end;
        };

        struct add_tokens {
            card_token_type token_type;
            int num_tokens;
            token_position target;
        };

        struct move_tokens {
            card_token_type token_type;
            int num_tokens;
            token_position origin;
            token_position target;
            animation_duration duration = durations.move_tokens;
        };

        struct move_train {
            int position;
            animation_duration duration = durations.move_train;
        };

        struct deck_shuffled {
            pocket_type pocket;
            animation_duration duration = durations.deck_shuffle;
        };

        struct show_card {
            card_ptr card;
            card_data info;
            animation_duration duration = durations.flip_card;
        };

        struct hide_card {
            card_ptr card;
            animation_duration duration = durations.flip_card;
        };

        struct tap_card {
            card_ptr card;
            bool inactive;
            animation_duration duration = durations.tap_card;
        };

        struct flash_card {
            card_ptr card;
            animation_duration duration = durations.flash_card;
        };

        struct short_pause {
            nullable_card card;
            animation_duration duration = durations.short_pause;
        };

        struct player_add {
            player_user_list players;
        };

        struct player_order {
            player_list players;
            animation_duration duration = durations.move_player;
        };

        struct player_hp {
            player_ptr player;
            int hp;
            animation_duration duration = durations.player_hp;
        };

        struct player_show_role {
            player_ptr player;
            player_role role;
            animation_duration duration = durations.flip_card;
        };

        struct player_flags {
            player_ptr player;
            banggame::player_flags flags;
        };

        struct switch_turn {
            struct transparent{};
            player_ptr player;
        };

        struct player_distance_item {
            player_ptr player;
            int value;
        };
    
        struct player_distances {
            std::vector<player_distance_item> distance_mods;
            int range_mod;
            int weapon_range;
        };

        struct timer_status {
            timer_id_t timer_id;
            game_duration duration;
        };

        struct request_status {
            nullable_card origin_card;
            nullable_player origin;
            nullable_player target;
            game_string status_text;
            playable_cards_list respond_cards;
            card_list highlight_cards;
            player_list target_set_players;
            card_list target_set_cards;
            player_distances distances;
            std::optional<timer_status> timer;
        };

        struct status_ready {
            playable_cards_list play_cards;
            player_distances distances;
        };

        struct game_flags {
            struct transparent{};
            banggame::game_flags flags;
        };

        struct play_sound {
            struct transparent{};
            sound_id sound;
        };

        struct status_clear {};

        struct clear_logs {};

    }

    using game_update = std::variant<
        game_updates::preload_assets,
        game_updates::game_error,
        game_updates::game_log,
        game_updates::game_prompt,
        game_updates::add_cards,
        game_updates::remove_cards,
        game_updates::move_card,
        game_updates::add_tokens,
        game_updates::move_tokens,
        game_updates::move_train,
        game_updates::deck_shuffled,
        game_updates::show_card,
        game_updates::hide_card,
        game_updates::tap_card,
        game_updates::flash_card,
        game_updates::short_pause,
        game_updates::player_add,
        game_updates::player_order,
        game_updates::player_hp,
        game_updates::player_show_role,
        game_updates::player_flags,
        game_updates::switch_turn,
        game_updates::request_status,
        game_updates::status_ready,
        game_updates::game_flags,
        game_updates::play_sound,
        game_updates::status_clear,
        game_updates::clear_logs
    >;

    struct modifier_pair {
        card_ptr card;
        target_list targets;
    };

    using modifier_list = std::vector<modifier_pair>;

    struct game_action {
        card_ptr card;
        modifier_list modifiers;
        target_list targets;
        bool bypass_prompt;
        std::optional<timer_id_t> timer_id;
    };
}

#endif