#include "thirst.h"

#include "game/game.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_thirst::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>(target_card, [](player_ptr origin, int &value) {
            if (value > 1) {
                value = 1;
            }
        });
    }
}