#include "doublebarrel.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void effect_doublebarrel::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                if (origin->get_card_sign(req->origin_card).suit == card_suit::diamonds) {
                    req->unavoidable = true;
                }
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
}