#ifndef __FRONTIER_CATTLE_DRIVE_H__
#define __FRONTIER_CATTLE_DRIVE_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_cattle_drive {
        game_string on_prompt(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(cattle_drive, effect_cattle_drive)
}

#endif