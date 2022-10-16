#include "horseshoe.h"

#include "../../game.h"

namespace banggame {
    
    void effect_horseshoe::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_num_checks>(target_card, [=](player *origin, int &num_checks) {
            if (origin == target) {
                ++num_checks;
            }
        });
    }
}