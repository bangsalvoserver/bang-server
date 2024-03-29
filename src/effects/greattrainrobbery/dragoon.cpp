#include "dragoon.h"

#include "game/game.h"

#include "effects/base/bang.h"

namespace banggame {

    void equip_dragoon::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::count_bangs_played>(origin_card, [=](player *target, int &value) {
            if (origin == target) {
                --value;
            }
        });
    }
}