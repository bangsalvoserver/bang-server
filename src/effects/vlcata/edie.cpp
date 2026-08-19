#include "edie.h"
#include "cards/game_events.h"
#include "game/game_table.h"
#include "effects/base/draw_check.h"

namespace banggame {
    void equip_edie::on_enable(card_ptr target_card, player_ptr target) {
        auto on_steal_destroy = [target, target_card](player_ptr origin, player_ptr victim, card_ptr card) {
            if (victim == target && origin != target && origin->alive()) {
                target->m_game->queue_request<request_draw_check>(
                    target_card, target,
                    [origin, target_card](card_sign sign) {
                        if (!sign.is_spades() && !origin->m_hand.empty()) {
                            origin->discard_card(random_element(origin->m_hand));
                        }
                    });
            }
        };

        target->m_game->add_listener<event_type::on_steal_card>(target_card, on_steal_destroy);
        target->m_game->add_listener<event_type::on_destroy_card>(target_card, on_steal_destroy);
    }
}