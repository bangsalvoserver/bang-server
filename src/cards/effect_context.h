#ifndef __CARDS_EFFECT_CONTEXT_H__
#define __CARDS_EFFECT_CONTEXT_H__

#include "card_enums.h"

namespace banggame {

    struct effect_context {
        serial::opt_card shopchoice = nullptr;
        serial::opt_player skipped_player = nullptr;
        int8_t discount = 0;
        bool ignore_distances = false;
        bool disable_banglimit = false;
        bool repeating = false;
    };

}

#endif