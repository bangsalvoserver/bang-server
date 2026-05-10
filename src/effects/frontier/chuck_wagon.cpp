#include "chuck_wagon.h"

#include "cards/game_events.h"

#include "effects/wildwestshow/ruleset.h"

#include "game/game_table.h"

namespace banggame {

    void equip_chuck_wagon::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>({ target_card, 30 }, [=, max_count=max_count](player_ptr origin, bool skipped) {
            if (origin == target) {
                int count = get_count_played_cards(origin);
                if ((!skipped || count) && count >= max_count) {
                    origin->heal(target_card, nullptr, 1);
                }
            }
        });
    }

}