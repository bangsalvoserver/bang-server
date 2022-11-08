#include "tequila_joe.h"

#include "game/game.h"

namespace banggame {
    
    void effect_tequila_joe::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_beer_modifier>(target_card, [target](player *origin, int &value) {
            if (target == origin) {
                ++value;
            }
        });
    }
}