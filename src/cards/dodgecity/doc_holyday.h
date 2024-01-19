#ifndef __DODGECITY_DOC_HOLYDAY_H__
#define __DODGECITY_DOC_HOLYDAY_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct handler_doc_holyday {
        bool on_check_target(card *origin_card, player *origin, const serial::card_list &target_cards, player *target) {
            return bot_suggestion::target_enemy{}.on_check_target(origin_card, origin, target);
        }
        void on_play(card *origin_card, player *origin, const serial::card_list &target_cards, player *target);
    };
}

#endif