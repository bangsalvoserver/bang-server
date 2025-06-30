#include "buntlinespecial.h"

#include "effects/base/bang.h"
#include "effects/base/requests.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    void effect_buntlinespecial::on_play(card_ptr origin_card, player_ptr origin) {
        event_card_key key{origin_card, 1};
        origin->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player_ptr e_origin, shared_request_bang) {
            if (origin == e_origin) {
                origin->m_game->add_listener<event_type::on_missed>(key, [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
                    if (req->target && req->origin == origin && flags.check(effect_flag::is_bang)) {
                        origin->m_game->queue_request<request_discard>(origin_card, e_origin, req->target, effect_flags{}, 0);
                    }
                });
                origin->m_game->queue_action([=]{
                    origin->m_game->remove_listeners(key);
                }, 90);
            }
        });
    }
}