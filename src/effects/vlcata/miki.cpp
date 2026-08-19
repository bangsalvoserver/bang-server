#include "miki.h"
#include "cards/game_events.h"
#include "effects/base/draw.h"
#include "effects/base/requests.h"
#include "game/game_table.h"

namespace banggame {
    void equip_miki::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_draw_cards>(target_card,
            [target](player_ptr origin, int &value) {
                if (origin == target) {
                    value = 4;
                }
            });

        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [target, target_card](player_ptr origin) {
                if (origin == target) {
                    target->m_game->queue_action([target, target_card]{
                        target->m_game->queue_request<request_discard_hand>(target_card, target, target);
                        target->m_game->queue_request<request_discard_hand>(target_card, target, target);
                    });
                }
            });
    }
}