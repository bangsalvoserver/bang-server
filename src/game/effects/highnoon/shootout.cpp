#include "shootout.h"

#include "../../game.h"

namespace banggame {

    void effect_shootout::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_bangs_played>({target_card, 1}, [](player *p, int &value) {
            --value;
        });
    }
}