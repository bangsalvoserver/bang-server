#ifndef __CANYONDIABLO_EQUIPS_H__
#define __CANYONDIABLO_EQUIPS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_packmule : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_indianguide : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_taxman : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_brothel : event_based_effect {
        static inline uint8_t effect_holder_counter = 0;

        void on_enable(card *target_card, player *target);
    };

    struct effect_bronco {
        void on_equip(card *target_card, player *target);
        void on_unequip(card *target_card, player *target);
    };

}

#endif