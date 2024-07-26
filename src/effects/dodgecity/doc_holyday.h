#ifndef __DODGECITY_DOC_HOLYDAY_H__
#define __DODGECITY_DOC_HOLYDAY_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct handler_doc_holyday {
        bool on_check_target(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
            return bot_suggestion::target_enemy{}.on_check_target(origin_card, origin, target);
        }
        void on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target);
    };

    DEFINE_MTH(doc_holyday, handler_doc_holyday)
}

#endif