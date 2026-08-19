#include "julka.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_julka::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card,
            [=](player_ptr origin, bool skipped) {
                if (origin == target && target->m_hp > 3) {
                    target_card->flash_card();
                    target->damage(target_card, nullptr, 1);
                }
            });
    }
}