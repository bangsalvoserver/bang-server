#ifndef __CANYONDIABLO_LASTWILL_H__
#define __CANYONDIABLO_LASTWILL_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct equip_lastwill : event_equip, bot_suggestion::target_friend {
        void on_enable(card *origin_card, player *origin);
    };

    struct handler_lastwill {
        bool on_check_target(card *origin_card, player *origin, const effect_target_list &targets) {
            return targets.empty() || bot_suggestion::target_friend{}.on_check_target(origin_card, origin, targets[0].target.get<target_type::player>());
        }
        game_string get_error(card *origin_card, player *origin, const effect_target_list &targets);
        void on_play(card *origin_card, player *origin, const effect_target_list &targets);
    };
}

#endif