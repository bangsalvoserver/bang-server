#include "herb_hunter.h"

#include "game/game_table.h"

#include "effects/base/death.h"

namespace banggame {
    
    void equip_herb_hunter::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_player_eliminated>({target_card, 2}, [p, target_card](player_ptr origin, player_ptr target, death_type type) {
            if (p != target) {
                p->m_game->queue_action([=]{
                    if (p->alive()) {
                        target_card->flash_card();
                        p->draw_card(2, target_card);
                    }
                });
            }
        });
    }
}