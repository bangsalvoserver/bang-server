#include "shade_oconnor.h"

#include "game/game.h"
#include "ruleset.h"

#include "effects/base/can_play_card.h"

namespace banggame {
    
    void equip_shade_oconnor::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({target_card, 1}, [=](player *target, shared_effect_context ctx) {
            if (origin != target) {
                origin->m_game->queue_request<request_can_play_card>(target_card, nullptr, origin, effect_flags{}, -1);
            }
        });
    }
}