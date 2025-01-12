#include "fistfulofcards.h"

#include "effects/base/bang.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_fistfulofcards::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::pre_turn_start>(origin_card, [=](player_ptr target) {
            if (!target->empty_hand()) {
                target->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", target, origin_card, int(target->m_hand.size()));
                for (int i = 0; i < target->m_hand.size(); ++i) {
                    target->m_game->queue_action([=]{
                        if (target->alive()) {
                            target->m_game->queue_request<request_bang>(origin_card, nullptr, target);
                        }
                    }, -1);
                }
            }
        });
    }
}