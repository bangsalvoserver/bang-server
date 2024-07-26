#include "generalstore.h"

#include "game/game.h"

namespace banggame {

    void request_generalstore::on_update() {
        if (!live) {
            target->play_sound("generalstore");
        }
        auto_pick();
    }

    void request_generalstore::on_pick(card_ptr target_card) {
        target->m_game->pop_request();
        target->m_game->add_log("LOG_DRAWN_FROM_GENERALSTORE", target, target_card, origin_card);
        target->add_to_hand(target_card);
    }

    game_string request_generalstore::status_text(player_ptr owner) const {
        if (target == owner) {
            return {"STATUS_GENERALSTORE", origin_card};
        } else {
            return {"STATUS_GENERALSTORE_OTHER", target, origin_card};
        }
    }

    void effect_generalstore::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        int num_targets = origin->m_game->num_alive() - bool(ctx.skipped_player);
        for (int i=0; i < num_targets; ++i) {
            origin->m_game->top_of_deck()->move_to(pocket_type::selection);
        }
    }

    void effect_generalstore::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_generalstore>(origin_card, origin, target);
    }
}