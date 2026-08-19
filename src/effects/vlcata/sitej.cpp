#include "sitej.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_sitej::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, -5},
            [target, target_card](player_ptr origin, bool skipped) {
                if (origin == target && !skipped) {
                    size_t count = target->m_hand.size();
                    while (!target->m_hand.empty()) {
                        target->discard_card(target->m_hand.front());
                    }
                    target_card->flash_card();
                    target->draw_card(static_cast<int>(count + 1), target_card);
                }
            });
    }
}