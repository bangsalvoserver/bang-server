#include "shotgun.h"

#include "game/game.h"
#include "cards/base/requests.h"

namespace banggame {
    
    void equip_shotgun::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::after_hit>({target_card, 3}, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin == p && target != p && bool(flags & effect_flags::is_bang)) {
                target->m_game->queue_action([=]{
                    if (target->alive() && !target->empty_hand()) {
                        target->m_game->queue_request<request_discard>(target_card, origin, target);
                    }
                });
            }
        });
    }
}