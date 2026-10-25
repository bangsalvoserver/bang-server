#include "verca.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_verca::on_enable(card_ptr target_card, player_ptr target) {
        // Priority must be lower than dodgecity ruleset's on_equip_card listener (priority 5),
        // which sets inactive=true, so Verca's override runs after it and actually wins.
        target->m_game->add_listener<event_type::on_equip_card>(target_card,
            [target](player_ptr origin, player_ptr owner, card_ptr card, const effect_context &ctx) {
                if (owner == target && card->is_green()) {
                    card->set_inactive(false);
                }
            });
    }
}