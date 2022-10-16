#include "doc_holyday.h"

#include "../../game.h"
#include "../base/bang.h"
#include "../base/steal_destroy.h"

namespace banggame {

    void handler_doc_holyday::on_play(card *origin_card, player *origin, tagged_value<target_type::cards> target_cards, player *target) {
        for (card *c : target_cards.value) {
            effect_destroy{}.on_play(origin_card, origin, c);
        }
        if (!std::ranges::all_of(target_cards.value, [&](card *c) {
            return target->immune_to(c, origin, {});
        })) {
            effect_bang{}.on_play(origin_card, origin, target);
        }
    }
}