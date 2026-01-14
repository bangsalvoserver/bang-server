#include "discount.h"

#include "game/game_table.h"

namespace banggame {

    bool modifier_discount::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return target_card->deck == card_deck_type::goldrush
            && target_card->pocket != pocket_type::player_table;
    }

    void modifier_discount::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.set<contexts::discount>(1);
    }
}