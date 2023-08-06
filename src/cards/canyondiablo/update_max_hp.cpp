#include "update_max_hp.h"

#include "game/game.h"

namespace banggame {

    void equip_update_max_hp::on_enable(card *target_card, player *target) {
        target->m_max_hp = value;
    }

    void equip_update_max_hp::on_disable(card *target_card, player *target) {
        target->reset_max_hp();
        target->set_hp(std::max(target->m_hp, target->m_max_hp));
    }
}