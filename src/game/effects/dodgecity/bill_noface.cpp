#include "bill_noface.h"

#include "../../game.h"

namespace banggame {

    void effect_bill_noface::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 2}, [target](player *origin, int &value) {
            if (target == origin) {
                value = 1 + target->m_max_hp - target->m_hp;
            }
        });
    }
}