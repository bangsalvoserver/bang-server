#include "discount.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_discount::verify(card *origin_card, player *origin, card *playing_card, effect_context &ctx) {
        --ctx.cost_difference;
        return {};
    }
}