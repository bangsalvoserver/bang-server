#include "discount.h"

#include "game/game.h"

namespace banggame {

    void modifier_discount::add_context(card *origin_card, player *origin, card *playing_card, effect_context &ctx) {
        ctx.discount = true;
    }
}