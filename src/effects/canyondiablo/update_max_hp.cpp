#include "update_max_hp.h"

#include "game/game.h"

namespace banggame {

    void equip_update_max_hp::on_enable(card_ptr target_card, player_ptr target) {
        target->m_max_hp = value;
    }

    void equip_update_max_hp::on_disable(card_ptr target_card, player_ptr target) {
        target->reset_max_hp();
        if (target->m_hp > target->m_max_hp && !target->is_ghost()) {
            target->set_hp(target->m_max_hp);
        }
    }
}