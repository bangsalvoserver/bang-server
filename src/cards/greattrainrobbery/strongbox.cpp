#include "strongbox.h"

#include "game/game.h"

namespace banggame {

    void equip_strongbox::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_turn_end>({origin_card, -4}, [=](player *target, bool skipped) {
            if (origin == target && target->alive()) {
                origin->draw_card(1, origin_card);
            }
        });
    }
}