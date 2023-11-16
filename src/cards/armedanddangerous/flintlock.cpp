#include "flintlock.h"

#include "cards/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void handler_flintlock::on_play(card *origin_card, player *origin, player *target, bool paid_cubes) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target, effect_flags::escapable | effect_flags::single_target);
        if (paid_cubes) {
            origin->m_game->add_listener<event_type::on_missed>(origin_card, [=](card *origin_card, player *p, player *target, effect_flags flags) {
                if (origin == p) {
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
        origin->m_game->queue_request(std::move(req));
    }
}