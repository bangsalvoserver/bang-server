#include "doc_holyday.h"

#include "effects/base/bang.h"
#include "effects/base/steal_destroy.h"

#include "game/game.h"

namespace banggame {

    void handler_doc_holyday::on_play(card *origin_card, player *origin, const serial::card_list &target_cards, player *target) {
        for (card *c : target_cards) {
            effect_discard{}.on_play(origin_card, origin, c);
        }
        if (!rn::all_of(target_cards, [&](card *target_card) {
            return target->immune_to(target_card, origin, {}, true);
        })) {
            effect_bang{}.on_play(origin_card, origin, target);
        } else {
            for (card *target_card : target_cards) {
                target->immune_to(target_card, origin, {});
            }
        }
    }
}