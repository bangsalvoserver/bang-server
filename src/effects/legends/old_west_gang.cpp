#include "old_west_gang.h"

#include "perform_feat.h"

#include "cards/game_enums.h"

#include "effects/base/damage.h"

#include "game/game.h"

namespace banggame {
    
    void equip_old_west_gang::on_enable(card_ptr origin_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(origin_card, [=](player_ptr origin) {
            event_card_key key{origin_card, 5};

            target->m_game->add_listener<event_type::on_hit>(key, [=, first_target = static_cast<player_ptr>(nullptr)](card_ptr e_origin_card, player_ptr e_origin, player_ptr e_target, int damage, effect_flags flags) mutable {
                if (origin == e_origin && e_target != origin) {
                    if (!first_target) {
                        first_target = e_target;
                    } else if (first_target != e_target) {
                        queue_request_perform_feat(origin_card, origin);
                    }
                }
            });

            target->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player_ptr origin, bool skipped) {
                target->m_game->remove_listeners(key);
            });
        });
    }
}