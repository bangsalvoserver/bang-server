#include "aim.h"

#include "game/game.h"
#include "game/possible_to_play.h"
#include "effects/base/bang.h"

namespace banggame {

    void effect_aim::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player_ptr p, shared_request_bang req) {
            if (p == origin) {
                ++req->bang_damage;
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
}