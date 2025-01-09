#include "old_west_gang.h"

#include "perform_feat.h"

#include "cards/game_enums.h"

#include "effects/base/damage.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_old_west_gang::on_enable(card_ptr origin_card, player_ptr target) {
        auto first_target = std::make_shared<player_ptr>(nullptr);

        target->m_game->add_listener<event_type::on_turn_start>(origin_card, [=](player_ptr origin) {
            *first_target = nullptr;
        });

        target->m_game->add_listener<event_type::on_hit>(origin_card, [=](card_ptr e_origin_card, player_ptr origin, player_ptr e_target, int damage, effect_flags flags) {
            if (origin && origin == target->m_game->m_playing && e_target != origin) {
                if (!(*first_target)) {
                    *first_target = e_target;
                } else if (*first_target != e_target) {
                    queue_request_perform_feat(origin_card, origin);
                }
            }
        });
    }
}