#include "pickaxe.h"

#include "../../game.h"

namespace banggame {

    void effect_pickaxe::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 1}, [=](player *origin, int &value) {
            if (origin == target) {
                ++value;
            }
        });
    }
}