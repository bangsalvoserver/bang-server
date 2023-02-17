#include "shopchoice.h"

#include "game/game.h"

namespace banggame {

    bool effect_shopchoice::can_play(card *origin_card, player *origin, const effect_context &ctx) {
        return ctx.repeating || ctx.shopchoice
            && ctx.shopchoice->get_tag_value(tag_type::shopchoice) == origin_card->get_tag_value(tag_type::shopchoice);
    }

    void modifier_shopchoice::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.shopchoice = origin_card;
    }
}