#ifndef __GREATTRAINROBBERY_TRAINCOST_H__
#define __GREATTRAINROBBERY_TRAINCOST_H__

#include "cards/card_effect.h"

namespace banggame {

    struct modifier_traincost {
        game_string get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };

    struct modifier_locomotive {
        bool valid_with_modifier(card *origin_card, player *origin, card *target_card);
        bool valid_with_card(card *origin_card, player *origin, card *target_card);
        void add_context(card *origin_card, player *origin, effect_context &ctx);
    };

}

#endif