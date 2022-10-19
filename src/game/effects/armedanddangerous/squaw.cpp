#include "squaw.h"

#include "../../game.h"
#include "../base/steal_destroy.h"

namespace banggame {

    void handler_squaw::on_play(card *origin_card, player *origin, card *target_card, opt_tagged_value<target_type::none> paid_cubes) {
        const auto flags = effect_flags::escapable | effect_flags::single_target;
        if (target_card->owner && !target_card->owner->immune_to(origin_card, origin, flags)) {
            if (paid_cubes) {
                effect_steal{}.on_play(origin_card, origin, target_card, flags);
            } else {
                effect_destroy{}.on_play(origin_card, origin, target_card, flags);
            }
        }
    }
}