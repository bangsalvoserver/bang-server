#include "slab_the_killer.h"

#include "game/game_table.h"
#include "bang.h"

namespace banggame {

    void equip_slab_the_killer::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(target_card, [p](player_ptr target, shared_request_bang req) {
            if (p == target) {
                ++req->bang_strength;
            }
        });
    }
}