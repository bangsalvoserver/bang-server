#include "flintlock.h"

#include "../../game.h"
#include "../base/requests.h"

namespace banggame {

    void handler_flintlock::on_play(card *origin_card, player *origin, player *target, opt_tagged_value<target_type::none> paid_cubes) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target, effect_flags::escapable | effect_flags::single_target);
        if (paid_cubes) {
            origin->m_game->add_listener<event_type::on_missed>(origin_card, [=](card *origin_card, player *p, player *target, bool is_bang) {
                if (origin == p) {
                    origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
                    origin->add_to_hand(origin_card);
                }
            });
            req->on_cleanup([=]{
                origin->m_game->remove_listeners(origin_card);
            });
        }
        origin->m_game->queue_request(std::move(req));
    }
}