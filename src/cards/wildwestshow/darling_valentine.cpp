#include "darling_valentine.h"

#include "cards/base/requests.h"

#include "game/game.h"

namespace banggame {

    void equip_darling_valentine::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, -1}, [=](player *origin) {
            int ncards = int(origin->m_hand.size());
            origin->m_game->queue_request<request_discard_hand>(target_card, origin);
            if (ncards > 0) {
                origin->m_game->queue_action([=]{
                    origin->draw_card(ncards, target_card);
                });
            }
        });
    }
}