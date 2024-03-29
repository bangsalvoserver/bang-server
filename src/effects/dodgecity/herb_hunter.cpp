#include "herb_hunter.h"

#include "game/game.h"

#include "effects/base/deathsave.h"

namespace banggame {
    
    void equip_herb_hunter::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_player_death>({target_card, 2}, [p, target_card](player *origin, player *target) {
            if (p != target) {
                p->m_game->queue_action([=]{
                    if (p->alive()) {
                        p->m_game->flash_card(target_card);
                        p->draw_card(2, target_card);
                    }
                });
            }
        });
    }
}