#include "henry_block.h"

#include "game/game.h"

#include "effects/base/bang.h"
#include "effects/base/steal_destroy.h"

namespace banggame {

    void equip_henry_block::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_destroy_card>(target_card, [=](player *origin, player *target, card *discarded_card) {
            if (p == target && p != origin) {
                p->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                target_card->flash_card();
                p->m_game->queue_request<request_bang>(target_card, target, origin);
            }
        });
    }
}