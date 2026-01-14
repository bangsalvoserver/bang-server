#include "flintlock.h"

#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void effect_flintlock::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (ctx.get<contexts::selected_cubes>().count(origin_card) != 0) {
            origin->m_game->add_listener<event_type::on_missed>(origin_card, [=](card_ptr missed_card, effect_flags flags, shared_request_bang req) {
                if (origin == req->origin) {
                    origin->m_game->queue_action([=]{
                        if (origin->alive()) {
                            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
                            origin->add_to_hand(origin_card);
                        }
                    }, 1);
                }
            });
            origin->m_game->queue_action([=]{
                origin->m_game->remove_listeners(origin_card);
            }, 90);
        }
    }
}