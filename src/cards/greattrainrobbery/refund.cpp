#include "refund.h"

#include "game/game.h"

#include "cards/base/steal_destroy.h"

namespace banggame {

    void equip_refund::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player *e_origin, player *target, card *target_card) {
            if (origin == target && e_origin != origin && origin_card != target_card) {
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {
                        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
                        origin->m_game->flash_card(origin_card);
                        origin->draw_card(1, origin_card);
                    }
                }, 41);
            }
        });
    }
}