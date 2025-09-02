#include "thirst.h"

#include "game/game_table.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_thirst::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::init_request_draw>(target_card, [](player_ptr origin, shared_request_draw req) {
            --req->num_cards_to_draw;
        });
    }
}