#include "refund.h"

#include "game/game.h"

#include "effects/base/steal_destroy.h"

namespace banggame {

    void equip_refund::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr e_origin, card_ptr target_card, bool is_destroyed, bool &handled) {
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