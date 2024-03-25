#ifndef __CANYONDIABLO_LASTWILL_H__
#define __CANYONDIABLO_LASTWILL_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_lastwill : event_equip, bot_suggestion::target_friend {
        game_string on_prompt(card *origin_card, player *origin, player *target);
        void on_enable(card *origin_card, player *origin);
    };

    struct handler_lastwill {
        bool on_check_target(card *origin_card, player *origin, const serial::card_list &target_cards, player *target) {
            return !target || bot_suggestion::target_friend{}.on_check_target(origin_card, origin, target);
        }
        
        bool can_play(card *origin_card, player *origin, const serial::card_list &target_cards, player *target);
        void on_play(card *origin_card, player *origin, const serial::card_list &target_cards, player *target);
    };

    DEFINE_MTH(lastwill, handler_lastwill)
}

#endif