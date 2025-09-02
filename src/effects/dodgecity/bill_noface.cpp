#include "bill_noface.h"

#include "game/game_table.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_bill_noface::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::init_request_draw>({target_card, 3}, [target](player_ptr origin, shared_request_draw req) {
            if (target == origin) {
                req->num_cards_to_draw = 1 + target->m_max_hp - target->m_hp;
            }
        });
    }
}