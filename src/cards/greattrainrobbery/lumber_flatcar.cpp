#include "lumber_flatcar.h"

#include "game/game.h"

namespace banggame {

    void equip_lumber_flatcar::on_enable(card *target_card, player *target) {
        --target->m_range_mod;
        target->send_player_status();
    }

    void equip_lumber_flatcar::on_disable(card *target_card, player *target) {
        ++target->m_range_mod;
        target->send_player_status();
    }
}