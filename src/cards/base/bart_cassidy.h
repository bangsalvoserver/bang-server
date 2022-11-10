#ifndef __BASE_BART_CASSIDY_H__
#define __BASE_BART_CASSIDY_H__

#include "cards/card_effect.h"

namespace banggame {

    struct equip_bart_cassidy : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif