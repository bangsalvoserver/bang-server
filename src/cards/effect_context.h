#ifndef __CARDS_EFFECT_CONTEXT_H__
#define __CARDS_EFFECT_CONTEXT_H__

#include "card_enums.h"

namespace banggame {

    struct effect_context {
        serial::opt_card card_choice = nullptr;
        serial::opt_player skipped_player = nullptr;
        serial::opt_card repeat_card = nullptr;
        serial::opt_card traincost = nullptr;
        int8_t train_advance = 0;
        int8_t locomotive_count = 0;
        int8_t discount = 0;
        bool ignore_distances = false;
        bool disable_banglimit = false;
    };

}

#endif