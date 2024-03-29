#ifndef __VALLEYOFSHADOWS_TUCO_FRANZISKANER_H__
#define __VALLEYOFSHADOWS_TUCO_FRANZISKANER_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct equip_tuco_franziskaner : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(tuco_franziskaner, equip_tuco_franziskaner)
}

#endif