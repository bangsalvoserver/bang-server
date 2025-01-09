#include "quick_death.h"
#include "cards/game_enums.h"

#include "effects/base/damage.h"

#include "game/game_table.h"

namespace banggame {

    void equip_quick_death::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>(origin_card, [=](card_ptr e_origin_card, player_ptr origin, player_ptr e_target, int damage, effect_flags flags) {
            if (origin == target->m_game->m_playing && e_target->m_hp + damage == e_target->m_max_hp && flags.check(effect_flag::is_bang)) {
                queue_request_perform_feat(origin_card, origin);
            }
        });
    }
}