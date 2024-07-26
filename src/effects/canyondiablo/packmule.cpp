#include "packmule.h"

#include "game/game.h"

namespace banggame {
    
    void equip_packmule::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::apply_maxcards_modifier>(target_card, [p](player_ptr origin, int &value) {
            if (origin == p) {
                ++value;
            }
        });
    }
}