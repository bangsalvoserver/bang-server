#include "shopchoice.h"

#include "cards/filter_enums.h"
#include "cards/effect_context.h"

#include "game/game.h"

namespace banggame {

    bool effect_shopchoice::can_play(card *origin_card, player *origin, const effect_context &ctx) {
        return ctx.repeat_card || ctx.shopchoice
            && ctx.shopchoice->get_tag_value(tag_type::shopchoice) == origin_card->get_tag_value(tag_type::shopchoice);
    }

    bool modifier_shopchoice::valid_with_card(card *origin_card, player *origin, card *target_card) {
        return target_card->pocket == pocket_type::hidden_deck
            && target_card->get_tag_value(tag_type::shopchoice) == origin_card->get_tag_value(tag_type::shopchoice);
    }

    void modifier_shopchoice::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.shopchoice = origin_card;
    }
}