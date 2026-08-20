#ifndef __VLCATA_SIMI_H__
#define __VLCATA_SIMI_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_simi_dynamite_master {};
    DEFINE_EQUIP(simi_dynamite_master, equip_simi_dynamite_master)

    struct effect_simi_take_dynamite {};
    DEFINE_EFFECT(simi_take_dynamite, effect_simi_take_dynamite)
}

#endif