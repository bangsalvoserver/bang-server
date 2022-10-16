#ifndef __VALLEYOFSHADOWS_TUCO_FRANZISKANER_H__
#define __VALLEYOFSHADOWS_TUCO_FRANZISKANER_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_tuco_franziskaner : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif