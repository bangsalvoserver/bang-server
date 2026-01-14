#include "belltower.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"

namespace banggame {

    bool modifier_belltower::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
        return playing_card->has_tag(tag_type::ranged_effect);
    }

    void modifier_belltower::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.add<contexts::ignore_distances>();
    }
}