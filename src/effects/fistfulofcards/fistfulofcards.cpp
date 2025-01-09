#include "fistfulofcards.h"

#include "game/game_table.h"
#include "effects/base/bang.h"

namespace banggame {
    
    void equip_fistfulofcards::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player_ptr p) {
            if (!p->empty_hand()) {
                p->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", p, target_card, int(p->m_hand.size()));
                for (int i=0; i<p->m_hand.size(); ++i) {
                    p->m_game->queue_request<request_bang>(target_card, nullptr, p, effect_flags{}, -1);
                }
            }
        });
    }
}