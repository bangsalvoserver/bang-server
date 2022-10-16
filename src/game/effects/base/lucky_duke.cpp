#include "lucky_duke.h"

#include "../../game.h"

namespace banggame {
    
    void effect_lucky_duke::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_num_checks>(target_card, [=](player *origin, int &num_checks) {
            if (origin == target) {
                ++num_checks;
            }
        });
    }
}