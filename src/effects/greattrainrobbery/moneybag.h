#ifndef __GREATTRAINROBBERY_MONEYBAG_H__
#define __GREATTRAINROBBERY_MONEYBAG_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_moneybag {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(moneybag, modifier_moneybag)
}

#endif