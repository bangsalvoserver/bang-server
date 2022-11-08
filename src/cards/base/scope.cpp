#include "scope.h"

#include "game/game.h"

namespace banggame {

    void effect_scope::on_enable(card *target_card, player *target) {
        ++target->m_range_mod;
        target->send_player_status();
    }

    void effect_scope::on_disable(card *target_card, player *target) {
        --target->m_range_mod;
        target->send_player_status();
    }
}