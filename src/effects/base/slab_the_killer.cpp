#include "slab_the_killer.h"

#include "game/game.h"
#include "bang.h"

namespace banggame {

    void equip_slab_the_killer::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(target_card, [p](player *target, shared_request_bang req) {
            if (p == target) {
                ++req->bang_strength;
            }
        });
    }
}