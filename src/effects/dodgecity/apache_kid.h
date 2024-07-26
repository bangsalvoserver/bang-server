#ifndef __GOLDRUSH_APACHE_KID_H__
#define __GOLDRUSH_APACHE_KID_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_apache_kid : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(apache_kid, equip_apache_kid)
}

#endif