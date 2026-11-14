// English comments only
#include "julka.h"

#include "cards/game_events.h"
#include "game/game_table.h"
#include "game/player.h"

namespace banggame {

    void equip_julka::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [=](player_ptr origin, bool skipped) {
            if (origin == target && origin->m_hp > 3 && origin->alive()) {
                origin->m_game->add_log("LOG_CARD_EFFECT", target, target_card);
                origin->damage(target_card, origin, 1);
            }
        });
    }

}