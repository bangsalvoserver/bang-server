#include "discount.h"

#include "game/game.h"

namespace banggame {

    void modifier_discount::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.discount = 1;
    }
}