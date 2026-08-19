#include "termit.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_termit::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_max_cards>(target_card,
            [target](const_player_ptr origin, int &value) {
                if (origin == target && target->m_hp < 3) {
                    ++value;
                }
            });
    }
}