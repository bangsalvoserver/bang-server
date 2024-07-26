#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "card_defs.h"

#include "game/request_base.h"

#include "vtable_effect.h"
#include "vtable_equip.h"
#include "vtable_modifier.h"
#include "vtable_mth.h"
#include "vtable_ruleset.h"

namespace banggame {

    struct event_equip {
        void on_disable(card_ptr target_card, player_ptr target);
    };

}


#endif