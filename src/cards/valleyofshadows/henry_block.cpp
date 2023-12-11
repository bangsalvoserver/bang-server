#include "henry_block.h"

#include "game/game.h"

#include "cards/base/bang.h"
#include "cards/base/steal_destroy.h"

namespace banggame {

    void equip_henry_block::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_destroy_card>(target_card, [=](player *origin, player *target, card *discarded_card) {
            if (p == target && p != origin) {
                p->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                p->m_game->flash_card(target_card);
                p->m_game->queue_request<request_bang>(target_card, target, origin);
            }
        });
    }
}