#include "man_with_no_name.h"

#include "perform_feat.h"

#include "effects/base/damage.h"

#include "game/game.h"

namespace banggame {

    void equip_man_with_no_name::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>(origin_card, [=](card_ptr e_origin_card, player_ptr origin, player_ptr e_target, int damage, effect_flags flags) {
            if (e_target == target->m_game->m_playing) {
                queue_request_perform_feat(origin_card, e_target);
            }
        });
    }
}