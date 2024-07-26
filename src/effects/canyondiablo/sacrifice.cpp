#include "sacrifice.h"

#include "game/game.h"
#include "effects/base/damage.h"

namespace banggame {

    bool effect_sacrifice::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_damage>()) {
            return req->target != origin && (!req->savior || req->savior == origin);
        }
        return false;
    }

    void effect_sacrifice::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_damage>();
        player_ptr saved = req->target;
        req->savior = origin;
        bool fatal = saved->m_hp <= req->damage;

        if (--req->damage == 0) {
            origin->m_game->pop_request();
        }
    
        origin->damage(origin_card, origin, 1);
        origin->m_game->queue_action([=]{
            if (origin->alive() && saved->alive()) {
                origin->draw_card(2 + fatal, origin_card);
            }
        });
    }
}