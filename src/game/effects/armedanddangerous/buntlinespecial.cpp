#include "buntlinespecial.h"

#include "../../game.h"
#include "../base/bang.h"
#include "../base/requests.h"

namespace banggame {

    void effect_buntlinespecial::on_play(card *origin_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *origin, request_bang *req) {
            if (p == origin) {
                p->m_game->add_listener<event_type::on_missed>(origin_card, [=](card *bang_card, player *origin, player *target, effect_flags flags) {
                    if (target && origin == p && bool(flags & effect_flags::is_bang) && !target->m_hand.empty()) {
                        target->m_game->queue_request<request_discard>(origin_card, origin, target);
                    }
                });
                req->on_cleanup([=]{
                    p->m_game->remove_listeners(origin_card);
                });
            }
        });
    }
}