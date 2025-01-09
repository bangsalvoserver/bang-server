#include "flintlock.h"

#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void effect_flintlock::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (ctx.selected_cubes.count(origin_card) != 0) {
            origin->m_game->add_listener<event_type::on_missed>(origin_card, [=](card_ptr origin_card, player_ptr p, player_ptr target, card_ptr missed_card, effect_flags flags) {
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
    }
}