#ifndef __GOLDRUSH_CALUMET_H__
#define __GOLDRUSH_CALUMET_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_calumet : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif