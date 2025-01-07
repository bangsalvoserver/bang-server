#ifndef __LEGENDS_LAST_HERO_H__
#define __LEGENDS_LAST_HERO_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_last_hero : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(last_hero, equip_last_hero)
}

#endif