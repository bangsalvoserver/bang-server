#include "packmule.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_packmule::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::apply_maxcards_modifier>(target_card, [p](const_player_ptr origin, int &value) {
            if (origin == p) {
                ++value;
            }
        });
    }
}