#include "generalstore.h"

#include "game/game.h"

namespace banggame {

    struct request_generalstore : selection_picker {
        request_generalstore(card *origin_card, player *origin, player *target)
            : selection_picker(origin_card, origin, target) {}

        void on_update() override {
            target->m_game->play_sound(target, "generalstore");
        }

        bool auto_resolve() override {
            return auto_pick();
        }

        void on_pick(card *target_card) override {
            auto lock = target->m_game->lock_updates(true);
            target->m_game->add_log("LOG_DRAWN_FROM_GENERALSTORE", target, target_card, origin_card);
            target->add_to_hand(target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_GENERALSTORE", origin_card};
            } else {
                return {"STATUS_GENERALSTORE_OTHER", target, origin_card};
            }
        }
    };

    void effect_generalstore::on_play(card *origin_card, player *origin) {
        for (int i=0; i<origin->m_game->num_alive(); ++i) {
            origin->m_game->move_card(origin->m_game->top_of_deck(), pocket_type::selection);
        }
    }

    void effect_generalstore::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->queue_request<request_generalstore>(origin_card, origin, target);
    }
}