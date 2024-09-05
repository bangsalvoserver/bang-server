#include "pickaxe.h"

#include "game/game.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_pickaxe::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 1}, [=](player_ptr origin, int &value) {
            if (origin == target) {
                ++value;
            }
        });
    }
}