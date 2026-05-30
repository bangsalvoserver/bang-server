#include "refund.h"

#include "game/game_table.h"

#include "effects/base/steal_destroy.h"

namespace banggame {

    void equip_refund::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_destroy_card>(origin_card, [=](player_ptr e_origin, card_ptr e_origin_card, card_ptr target_card, destroy_flags &flags) {
            if (origin == target_card->owner && e_origin != origin && origin_card != target_card && flags.check(destroy_flag::intentional)) {
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