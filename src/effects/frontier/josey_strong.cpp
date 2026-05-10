#include "josey_strong.h"

#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void effect_josey_strong::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player_ptr e_origin, shared_request_bang req) {
            if (e_origin == origin) {
                req->unavoidable = true;
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
}