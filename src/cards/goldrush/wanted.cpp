#include "wanted.h"

#include "game/game.h"

namespace banggame {

    void equip_wanted::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_player_death>({target_card, 3}, [p, target_card](player *origin, player *target) {
            if (origin && p == target && origin != target) {
                origin->m_game->flash_card(target_card);
                origin->draw_card(2, target_card);
                origin->add_gold(1);
            }
        });
    }
}