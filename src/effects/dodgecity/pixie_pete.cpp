#include "pixie_pete.h"

#include "game/game_table.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_pixie_pete::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 2}, [=](player_ptr origin, int &value) {
            if (origin == target && value == 2) {
                value = 3;
            }
        });
    }
}