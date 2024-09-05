#include "bill_noface.h"

#include "game/game.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_bill_noface::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 3}, [target](player_ptr origin, int &value) {
            if (target == origin) {
                value = 1 + target->m_max_hp - target->m_hp;
            }
        });
    }
}