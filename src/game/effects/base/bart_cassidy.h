#ifndef __BASE_BART_CASSIDY_H__
#define __BASE_BART_CASSIDY_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_bart_cassidy : event_based_effect {
        void on_enable(card *target_card, player *target);
    };
}

#endif