#ifndef __GOLDRUSH_APACHE_KID_H__
#define __GOLDRUSH_APACHE_KID_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_apache_kid : event_equip {
        void on_enable(card *target_card, player *target);
    };

    DEFINE_EQUIP(apache_kid, equip_apache_kid)
}

#endif