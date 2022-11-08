#include "initialcards.h"

#include "game/game.h"

namespace banggame {
    
    void effect_initialcards::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::apply_initial_cards_modifier>(target_card, [target, value=value](player *p, int &value_ref) {
            if (p == target) {
                value_ref = value;
            }
        });
    }
}