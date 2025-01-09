#include "buntlinespecial.h"

#include "effects/base/bang.h"
#include "effects/base/requests.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    void effect_buntlinespecial::on_play(card_ptr origin_card, player_ptr p) {
        event_card_key key{origin_card, 1};
        p->m_game->add_listener<event_type::apply_bang_modifier>(key, [=](player_ptr origin, shared_request_bang req) {
            if (p == origin) {
                p->m_game->add_listener<event_type::on_missed>(key, [=](card_ptr bang_card, player_ptr origin, player_ptr target, card_ptr missed_card, effect_flags flags) {
                    if (target && origin == p && flags.check(effect_flag::is_bang)) {
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