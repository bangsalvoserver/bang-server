#include "shopchoice.h"

#include "game/game.h"

namespace banggame {

    game_string effect_shopchoice::verify(card *origin_card, player *origin, const effect_context &ctx) {
        if (!ctx.repeating && origin_card->get_tag_value(tag_type::shopchoice).value_or(0) != ctx.shopchoice) {
            return "ERROR_INVALID_SHOPCHOICE";
        } else {
            return {};
        }
    }

    void modifier_shopchoice::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.shopchoice = origin_card->get_tag_value(tag_type::shopchoice).value_or(0);
    }
}