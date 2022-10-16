#include "tuco_franziskaner.h"

#include "../../game.h"

namespace banggame {

    void effect_tuco_franziskaner::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 1}, [p](player *origin, int &value) {
            if (p == origin && std::ranges::find(p->m_table, card_color_type::blue, &card::color) == p->m_table.end()) {
                value += 2;
            }
        });
    }
}