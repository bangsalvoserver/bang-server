#include "lasso.h"

#include "game/game.h"

namespace banggame {
    
    void equip_lasso::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](const card *c) {
            return c->pocket == pocket_type::player_table;
        });
    }

    void equip_lasso::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
    }
}