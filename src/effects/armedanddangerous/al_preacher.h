#ifndef __ARMEDANDDANGEROUS_AL_PREACHER_H__
#define __ARMEDANDDANGEROUS_AL_PREACHER_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_al_preacher : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(al_preacher, equip_al_preacher)
}

#endif