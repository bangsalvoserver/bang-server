#include "thunderer.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void effect_thunderer::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                req->origin->m_game->add_log("LOG_STOLEN_SELF_CARD", req->origin, req->origin_card);
                req->origin->m_game->move_card(req->origin_card, pocket_type::player_hand, req->origin, show_card_flags::short_pause);
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
}