#include "dragoon.h"

#include "game/game.h"

#include "effects/base/bang.h"

namespace banggame {

    void equip_dragoon::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::count_bangs_played>(origin_card, [=](player_ptr target, int &value) {
            if (origin == target) {
                --value;
            }
        });
    }
}