#include "add_gold.h"

#include "game/game.h"

namespace banggame {

    void effect_add_gold::on_play(card *origin_card, player *origin, player *target) {
        target->add_gold(amount);
    }
}