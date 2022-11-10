#include "greg_digger.h"

#include "game/game.h"

namespace banggame {

    void equip_greg_digger::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_player_death>(target_card, [p](player *origin, player *target) {
            if (p != target) {
                p->heal(2);
            }
        });
    }
}