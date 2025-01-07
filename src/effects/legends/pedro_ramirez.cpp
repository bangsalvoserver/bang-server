#include "pedro_ramirez.h"

#include "effects/base/draw.h"

#include "game/game.h"

namespace banggame {

    void equip_pedro_ramirez_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [=](player_ptr origin, card_ptr drawn_card, shared_request_draw req_draw, bool &reveal) {
            if (origin == target && req_draw->num_drawn_cards == 1) {
                target->m_game->queue_action([=]{
                    if (!target->m_game->m_discards.empty()) {
                        effect_draw_discard{}.on_play(target_card, origin);
                    }
                }, -20);
            }
        });
    }
}