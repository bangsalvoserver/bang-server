#ifndef __BASE_CALAMITY_JANET_H__
#define __BASE_CALAMITY_JANET_H__

#include "cards/card_effect.h"

#include "cards/game_enums.h"
namespace banggame {

    struct equip_calamity_janet {
        player_flags flag;
        equip_calamity_janet(int value)
            : flag{value == 1 ? player_flags::treat_any_as_bang : player_flags::treat_missed_as_bang} {}
        
        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

    struct handler_play_as_missed {
        bool can_play(card *origin_card, player *origin, card *target_card);
        game_string on_prompt(card *origin_card, player *origin, card *target_card);
        void on_play(card *origin_card, player *origin, card *target_card);
    };
}

#endif