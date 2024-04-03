#include "flintlock.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/filters.h"

namespace banggame {

    void effect_flintlock::on_play(card *origin_card, player *origin, player *target, const effect_context &ctx) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target, effect_flags::escapable | effect_flags::single_target);
        if (!filters::get_selected_cubes(origin_card, ctx).empty()) {
            origin->m_game->add_listener<event_type::on_missed>(origin_card, [=](card *origin_card, player *p, player *target, card *missed_card, effect_flags flags) {
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