#include "judge.h"

#include "../../game.h"

namespace banggame {

    void effect_judge::on_enable(card *target_card, player *target) {
        target->m_game->set_game_flags(game_flags::disable_equipping);
    }
}