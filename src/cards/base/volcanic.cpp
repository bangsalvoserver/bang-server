#include "volcanic.h"

#include "game/game.h"

#include "bang.h"

namespace banggame {
    
    void equip_volcanic::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_bangs_played>(target_card, [=](player *origin, int &value) {
            if (origin == target) {
                value = 0;
            }
        });
    }
}