#ifndef __TARGET_TYPE_CARD_H__
#define __TARGET_TYPE_CARD_H__

#include "cards/card_effect.h"
#include "cards/game_enums.h"

#include "game/possible_to_play.h"

namespace banggame {

    struct targeting_card : target_args::card {
        using value_type = card_ptr;

        targeting_card(target_args::card args) : target_args::card{args} {}

        const auto &get_args() const {
            return static_cast<const target_args::card &>(*this);
        }
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
            return get_all_targetable_cards(origin)
                | rv::filter([=, &ctx, this](card_ptr target_card) {
                    return !get_error(origin_card, origin, effect, ctx, target_card);
                });
        }

        scored_target<card_ptr> random_target(this const auto &self, card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return random_max_element(
                self.possible_targets(origin_card, origin, effect, ctx),
                [&](card_ptr target_card) {
                    if (effect.on_prompt(origin_card, origin, target_card, effect_flag::single_target, ctx)) return -2;
                    return effect.evaluate(origin_card, origin, target_card, ctx);
                },
                origin->m_game->bot_rng
            );
        }

        game_string get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) const;
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) const;
        void add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, card_ptr target) const;
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) const;
    };

    DEFINE_TARGETING(card, targeting_card)
}

#endif