#include "herb_hunter.h"

#include "game/game.h"

namespace banggame {
    
    void effect_herb_hunter::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_player_death>(target_card, [p, target_card](player *origin, player *target) {
            if (p != target) {
                p->m_game->flash_card(target_card);
                p->draw_card(2, target_card);
            }
        });
    }
}