#include "slab_the_killer.h"

#include "perform_feat.h"

#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void equip_slab_the_killer_legend::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player_ptr target, shared_request_bang req) {
            if (origin == target) {
                req->unavoidable = true;
            }
        });

        origin->m_game->add_listener<event_type::check_damage_legend_kill>(origin_card, [=](player_ptr target) {
            return origin == target;
        });
    }
}