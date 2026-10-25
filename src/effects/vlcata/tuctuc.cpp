#include "tuctuc.h"

#include "effects/base/heal.h"

#include "game/game_table.h"

namespace banggame {

    void equip_tuctuc::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_heal>(target_card, [target, target_card](card_ptr origin_card, player_ptr origin, player_ptr e_target, int amount) {
            if (e_target == target) {
                target->m_game->queue_action([target, target_card]{
                    if (target->alive()) {
                        target_card->flash_card();
                        target->draw_card(2, target_card);
                    }
                });
            }
        });
    }

}
