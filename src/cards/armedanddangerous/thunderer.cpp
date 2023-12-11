#include "thunderer.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void effect_thunderer::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 1};
        origin->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player *p, shared_request_bang req) {
            if (p == origin) {
                req->origin->m_game->add_log("LOG_STOLEN_SELF_CARD", req->origin, req->origin_card);
                req->origin->m_game->add_short_pause(req->origin_card);
                req->origin->add_to_hand(req->origin_card);
                origin->m_game->remove_listeners(key);
            }
        });
    }
}