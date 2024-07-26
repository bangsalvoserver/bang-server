#ifndef __ARMEDANDDANGEROUS_BANDOLIER_H__
#define __ARMEDANDDANGEROUS_BANDOLIER_H__

#include "cards/card_effect.h"
#include "effects/base/bang.h"

namespace banggame {

    struct modifier_bandolier : modifier_bangmod {
        bool valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card);
        game_string on_prompt(card_ptr origin_card, player_ptr origin, card_ptr playing_card);
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx);
    };

    DEFINE_MODIFIER(bandolier, modifier_bandolier)
}

#endif