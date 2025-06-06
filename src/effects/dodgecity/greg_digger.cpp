#include "greg_digger.h"

#include "game/game_table.h"

#include "effects/base/death.h"

namespace banggame {

    void equip_greg_digger::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_player_eliminated>({target_card, 1}, [p](player_ptr origin, player_ptr target, death_type type) {
            if (p != target && type != death_type::shadow_turn_end) {
                p->m_game->queue_action([=]{
                    if (p->alive()) {
                        p->heal(2);
                    }
                });
            }
        });
    }
}