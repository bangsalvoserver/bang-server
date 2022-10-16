#include "henry_block.h"

#include "../../game.h"
#include "../base/bang.h"

namespace banggame {

    void effect_henry_block::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_discard_card>(target_card, [=](player *origin, player *target, card *discarded_card) {
            if (p == target && p != origin) {
                p->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                p->m_game->queue_request<request_bang>(target_card, target, origin);
            }
        });
    }
}