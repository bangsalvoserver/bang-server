#include "shootout.h"

#include "game/game.h"

namespace banggame {

    void equip_shootout::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_bangs_played>({target_card, 1}, [](player *p, int &value) {
            --value;
        });
    }
}