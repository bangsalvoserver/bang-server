#include "al_preacher.h"

#include "game/game.h"

#include "effects/base/can_play_card.h"

namespace banggame {

    void equip_al_preacher::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_equip_card>(target_card, [=](player *origin, player *target, card *equipped_card, const effect_context &ctx) {
            if (p != origin && (equipped_card->is_blue() || equipped_card->is_orange())) {
                p->m_game->queue_request<request_can_play_card>(target_card, origin, p);
            }
        });
    }

}