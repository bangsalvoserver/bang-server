#include "miki.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_miki::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [target, target_card](player_ptr origin) {
                if (origin == target) {
                    target->draw_card(2, target_card);
                }
            });
    }
}