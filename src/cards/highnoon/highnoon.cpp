#include "highnoon.h"

#include "game/game.h"

namespace banggame {

    void effect_highnoon::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            p->damage(target_card, nullptr, 1);
        });
    }
}