#include "discount.h"

#include "game/game.h"

namespace banggame {

    bool modifier_discount::valid_with_card(card *origin_card, player *origin, card *target_card) {
        return target_card->deck == card_deck_type::goldrush
            && target_card->pocket != pocket_type::player_table;
    }

    void modifier_discount::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.discount = 1;
    }
}