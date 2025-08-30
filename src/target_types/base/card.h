#ifndef __TARGET_TYPE_CARD_H__
#define __TARGET_TYPE_CARD_H__

#include "cards/card_effect.h"

#include "game/possible_to_play.h"

namespace banggame {

    using targeting_card_args = targeting_args<void, target_filter::card>;

    struct targeting_card : targeting_card_args {
        using value_type = card_ptr;

        targeting_card(targeting_card_args args) : targeting_card_args{args} {}

        const auto &get_args() const {
            return static_cast<const targeting_card_args &>(*this);
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return get_all_targetable_cards(origin)
                | rv::filter([=, &ctx, this](card_ptr target_card) {
                    return !get_error(origin_card, origin, effect, ctx, target_card);
                });
        }

        card_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return random_element(possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng);
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, card_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target);
    };

    DEFINE_TARGETING(card, targeting_card)
}

#endif