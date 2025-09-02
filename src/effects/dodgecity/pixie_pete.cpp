#include "pixie_pete.h"

#include "game/game_table.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_pixie_pete::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::init_request_draw>({target_card, 2}, [=](player_ptr origin, shared_request_draw req) {
            if (origin == target && req->num_cards_to_draw == 2) {
                req->num_cards_to_draw = 3;
            }
        });
    }
}