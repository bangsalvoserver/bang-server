#include "shotgun.h"

#include "effects/base/requests.h"
#include "effects/base/damage.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_shotgun::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 3}, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin == p && target != p && flags.check(effect_flag::is_bang)) {
                target->m_game->queue_request<request_discard>(target_card, origin, target, effect_flags{}, 0);
            }
        });
    }
}