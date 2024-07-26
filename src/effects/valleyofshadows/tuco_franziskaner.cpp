#include "tuco_franziskaner.h"

#include "game/game.h"

#include "effects/base/draw.h"

namespace banggame {

    void equip_tuco_franziskaner::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 1}, [p](player_ptr origin, int &value) {
            if (p == origin && rn::none_of(p->m_table, &card::is_blue)) {
                value += 2;
            }
        });
    }
}