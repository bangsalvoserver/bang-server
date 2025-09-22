#ifndef __TARGET_TYPE_RANDOM_IF_HAND_CARD_H__
#define __TARGET_TYPE_RANDOM_IF_HAND_CARD_H__

#include "card.h"

namespace banggame {

    struct targeting_random_if_hand_card : targeting_card {
        using targeting_card::targeting_card;
        
        auto possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return get_all_targetable_cards(origin)
                | rv::filter([=, &ctx, this, seen_players=player_set{}](card_ptr target_card) mutable {
                    if (target_card->pocket == pocket_type::player_hand) {
                        if (seen_players.contains(target_card->owner) || get_error(origin_card, origin, effect, ctx, target_card)) {
                            return false;
                        }
                        seen_players.add(target_card->owner);
                        return true;
                    }
                    return !get_error(origin_card, origin, effect, ctx, target_card);
                });
        }

        card_ptr random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
            return random_element(possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng);
        }
        
        void on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
            if (target->pocket == pocket_type::player_hand) {
                target = target->owner->random_hand_card();
            }
            targeting_card::on_play(origin_card, origin, effect, ctx, target);
        }
    };

    DEFINE_TARGETING(random_if_hand_card, targeting_random_if_hand_card)
}

#endif