#ifndef __TARGET_TYPE_CARDS_H__
#define __TARGET_TYPE_CARDS_H__

#include "card.h"

namespace banggame {

    struct targeting_cards {
        using value_type = card_list;

        targeting_card target_card;
        int ncards;

        targeting_cards(targeting_args<int, target_filter::card> args)
            : target_card{{ .player_filter = args.player_filter, .card_filter = args.card_filter }}
            , ncards{args.target_value} {}

        auto get_args() const {
            struct args {
                player_filter_bitset player_filter;
                card_filter_bitset card_filter;
                int ncards;
            };
            return args{ target_card.player_filter, target_card.card_filter, ncards };
        }
        
        bool is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        card_list random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx);
        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target);
    };

    DEFINE_TARGETING(cards, targeting_cards)
}

#endif