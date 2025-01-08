#include "miss_susanna.h"

#include "ruleset.h"

#include "game/game.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1}, [=](player_ptr origin, bool skipped) {
            int count = 0;
            target->m_game->call_event(event_type::get_count_played_cards{ origin, count });
            if ((!skipped || count) && count < 3) {
                origin->damage(target_card, nullptr, 1);
            }
        });
    }
}