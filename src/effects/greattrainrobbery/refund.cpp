#include "refund.h"

#include "game/game.h"

#include "effects/base/steal_destroy.h"

namespace banggame {

    void equip_refund::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player *e_origin, card *target_card, bool &handled) {
            if (origin == target_card->owner && e_origin != origin && origin_card != target_card) {
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {
                        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
                        origin_card->flash_card();
                        origin->draw_card(1, origin_card);
                    }
                }, 41);
            }
        });
    }
}