#ifndef __GOLDRUSH_APACHE_KID_H__
#define __GOLDRUSH_APACHE_KID_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_apache_kid : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif