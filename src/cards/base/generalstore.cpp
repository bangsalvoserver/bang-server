#include "generalstore.h"

#include "game/game.h"

namespace banggame {

    void request_generalstore::on_update() {
        if (!sent) {
            target->m_game->play_sound(target, "generalstore");
        }
        auto_pick();
    }

    void request_generalstore::on_pick(card *target_card) {
        target->m_game->invoke_action([&]{
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DRAWN_FROM_GENERALSTORE", target, target_card, origin_card);
            target->add_to_hand(target_card);
        });
    }

    game_string request_generalstore::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_GENERALSTORE", origin_card};
        } else {
            return {"STATUS_GENERALSTORE_OTHER", target, origin_card};
        }
    }

    void effect_generalstore::on_play(card *origin_card, player *origin, player *target) {
        if (step == 1) {
            origin->m_game->move_card(origin->m_game->top_of_deck(), pocket_type::selection);
        } else if (step == 2) {
            origin->m_game->queue_request<request_generalstore>(origin_card, origin, target);
        }
    }
}