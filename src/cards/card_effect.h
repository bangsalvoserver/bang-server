#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "card_defs.h"

#include "game/game_string.h"
#include "game/request_base.h"

#include "vtable_effect.h"
#include "vtable_equip.h"
#include "vtable_modifier.h"
#include "vtable_mth.h"

namespace banggame {

    struct event_equip {
        void on_disable(card *target_card, player *target);
    };

}


#endif