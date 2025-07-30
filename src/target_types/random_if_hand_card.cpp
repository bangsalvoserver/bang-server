#include "random_if_hand_card.h"

#include "game/possible_to_play.h"

namespace banggame {

    void targeting_random_if_hand_card::on_play(const effect_context &ctx, card_ptr target) {
        if (target->pocket == pocket_type::player_hand) {
            targeting_card::on_play(ctx, target->owner->random_hand_card());
        } else {
            targeting_card::on_play(ctx, target);
        }
    }

}