#include "packmule.h"

#include "game/game.h"

namespace banggame {
    
    void effect_packmule::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::apply_maxcards_modifier>(target_card, [p](player *origin, int &value) {
            if (origin == p) {
                ++value;
            }
        });
    }
}