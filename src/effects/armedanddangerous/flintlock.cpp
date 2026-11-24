#include "flintlock.h"

#include "effects/base/bang.h"
#include "effects/base/missed.h"

#include "game/game_table.h"

namespace banggame {

    int effect_flintlock::evaluate(card_ptr origin_card, player_ptr origin, int ncubes, const effect_context &ctx) const {
        if (ncubes != 0) {
            if (player_ptr target = ctx.get<contexts::selected_player>()) {
                if (target->empty_hand() && count_missed_cards(target) == 0) {
                    return -5;
                }
            }
        }
        return ncubes;
    }

    void effect_flintlock::on_play(card_ptr origin_card, player_ptr origin, int ncubes) const {
        if (ncubes != 0) {
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