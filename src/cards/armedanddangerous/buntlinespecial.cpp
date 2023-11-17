#include "buntlinespecial.h"

#include "cards/base/bang.h"
#include "cards/base/requests.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void effect_buntlinespecial::on_play(card *origin_card, player *p) {
        event_card_key key{origin_card, 1};
        p->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player *origin, request_bang *req) {
            if (p == origin) {
                p->m_game->add_listener<event_type::on_missed>(key, [=](card *bang_card, player *origin, player *target, card *missed_card, effect_flags flags) {
                    if (target && origin == p && bool(flags & effect_flags::is_bang)) {
                        target->m_game->queue_request<request_discard>(origin_card, origin, target, effect_flags{}, 0);
                    }
                });
                origin->m_game->queue_action([=]{
                    origin->m_game->remove_listeners(key);
                }, 90);
            }
        });
    }
}