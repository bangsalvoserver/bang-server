#include "darling_valentine.h"

#include "effects/base/requests.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    struct request_darling_valentine : request_discard_hand {
        using request_discard_hand::request_discard_hand;

        int ncards = 0;

        void on_update() override {
            if (!live) {
                ncards = int(target->m_hand.size());
            }
            request_discard_hand::on_update();
        }

        void on_resolve() override {
            request_discard_hand::on_resolve();
            if (ncards > 0) {
                target->draw_card(ncards, origin_card);
            }
        }
    };

    void equip_darling_valentine::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, -1}, [=](player_ptr origin) {
            origin->m_game->queue_request<request_darling_valentine>(target_card, origin);
        });
    }
}