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

    struct [[=json::transparent]] card_response_pair {
        card_ptr card;
        [[=json::ignore]] bool is_response;
    };

    using card_response_list = std::vector<card_response_pair>;

    struct playable_card_info {
        card_ptr card;
        [[=json::ignore]] bool is_response;
        card_response_list modifiers;
        effect_context context;
    };

    using playable_cards_list = std::vector<playable_card_info>;
    
    using timer_id_t = uint16_t;

    namespace token_positions {
        struct table {};

        struct [[=json::transparent]] card {
            card_ptr card;
        };

        struct [[=json::transparent]] player {
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

        struct [[=json::transparent]] game_error {
            game_string message;

            game_error(game_string message)
                : message{message} {}
        };

        struct [[=json::transparent]] game_log {
            game_string message;

            game_log(game_string message)
                : message{message} {}
        };

        struct [[=json::transparent]] game_prompt {
            game_string message;

            game_prompt(game_string message)
                : message{message} {}
        };

        struct add_cards {
            card_backface_list card_ids;
            pocket_type pocket;
            nullable_player player;

            add_cards(card_backface_list card_ids, pocket_type pocket, nullable_player player = nullptr)
                : card_ids{std::move(card_ids)}, pocket{pocket}, player{player} {}
        };

        struct remove_cards {
            card_list cards;

            remove_cards(card_list cards)
                : cards{std::move(cards)} {}
        };

        struct move_card {
            card_ptr card;
            nullable_player player;
            pocket_type pocket;
            pocket_position position;
            animation_duration duration;

            move_card(card_ptr card, nullable_player player, pocket_type pocket, pocket_position position = pocket_position::end, bool instant = false)
                : card{card}, player{player}, pocket{pocket}, position{position}
                , duration{instant ? durations.instant : durations.move_card} {}
        };

        struct add_tokens {
            card_token_type token_type;
            int num_tokens;
            token_position target;

            add_tokens(card_token_type token_type, int num_tokens, token_position target)
                : token_type{token_type}, num_tokens{num_tokens}, target{target} {}
        };

        struct move_tokens {
            card_token_type token_type;
            int num_tokens;
            token_position origin;
            token_position target;
            animation_duration duration;

            move_tokens(card_token_type token_type, int num_tokens, token_position origin, token_position target, bool instant = false)
                : token_type{token_type}, num_tokens{num_tokens}, origin{origin}, target{target}
                , duration{instant ? durations.instant : num_tokens == 1 ? durations.move_token : durations.move_tokens} {}
        };

        struct move_train {
            int position;
            animation_duration duration;

            move_train(int position, bool instant = false)
                : position{position}
                , duration{instant ? durations.instant : durations.move_train} {}
        };

        struct deck_shuffled {
            pocket_type pocket;
            animation_duration duration;

            deck_shuffled(pocket_type pocket, bool instant = false)
                : pocket{pocket}
                , duration{instant ? durations.instant : durations.deck_shuffle} {}
        };

        struct show_card {
            card_ptr card;
            card_data info;
            animation_duration duration;

            show_card(card_ptr card, const card_data &info, bool instant = false)
                : card{card}, info{info}
                , duration{instant ? durations.instant : durations.flip_card} {}
        };

        struct hide_card {
            card_ptr card;
            animation_duration duration;

            hide_card(card_ptr card, bool instant = false)
                : card{card}
                , duration{instant ? durations.instant : durations.flip_card} {}
        };

        struct exchange_card {
            card_ptr card;
            card_ptr new_card;
            card_data info;
            animation_duration duration;

            exchange_card(card_ptr card, card_ptr new_card, const card_data &info, bool instant = false)
                : card{card}, new_card{new_card}, info{info}
                , duration{instant ? durations.instant : durations.flip_card} {}
        };

        struct tap_card {
            card_ptr card;
            bool inactive;
            animation_duration duration;

            tap_card(card_ptr card, bool inactive, bool instant = false)
                : card{card}, inactive{inactive}
                , duration{instant ? durations.instant : durations.tap_card} {}
        };

        struct flash_card {
            card_list cards;
            animation_duration duration;

            flash_card(card_list cards, bool instant = false)
                : cards{std::move(cards)}
                , duration{instant ? durations.instant : durations.flash_card} {}
        };

        struct short_pause {
            nullable_card card;
            animation_duration duration;

            short_pause(nullable_card card = nullptr, bool instant = false)
                : card{card}
                , duration{instant ? durations.instant : durations.short_pause} {}
        };

        struct player_add {
            player_user_list players;

            player_add(player_user_list players)
                : players{std::move(players)} {}
        };

        struct player_order {
            player_list players;
            animation_duration duration;

            player_order(player_list players, bool instant = false)
                : players{std::move(players)}
                , duration{instant ? durations.instant : durations.move_player} {}
        };

        struct player_hp {
            player_ptr player;
            int hp;
            animation_duration duration;

            player_hp(player_ptr player, int hp, bool instant = false)
                : player{player}, hp{hp}
                , duration{instant ? durations.instant : durations.player_hp} {}
        };

        struct player_show_role {
            player_ptr player;
            player_role role;
            animation_duration duration;

            player_show_role(player_ptr player, player_role role, bool instant = false)
                : player{player}, role{role}
                , duration{instant ? durations.instant : durations.flip_card} {}
        };

        struct player_flags {
            player_ptr player;
            banggame::player_flags flags;

            player_flags(player_ptr player, banggame::player_flags flags)
                : player{player}, flags{flags} {}
        };

        struct [[=json::transparent]] switch_turn {
            player_ptr player;

            switch_turn(player_ptr player)
                : player{player} {}
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

        struct [[=json::transparent]] game_flags {
            banggame::game_flags flags;

            game_flags(banggame::game_flags flags)
                : flags{flags} {}
        };

        struct [[=json::transparent]] play_sound {
            sound_id sound;

            play_sound(sound_id sound)
                : sound{sound} {}
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
        game_updates::exchange_card,
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
}

#endif