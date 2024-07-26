#include "shotgun.h"

#include "effects/base/requests.h"
#include "effects/base/damage.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_shotgun::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 3}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin == p && target != p && flags.check(effect_flag::is_bang)) {
                target->m_game->queue_request<request_discard>(target_card, origin, target, effect_flags{}, 0);
            }
        });
    }
}