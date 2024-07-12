#include "judge.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void equip_judge::on_enable(card *target_card, player *target) {
        target->m_game->add_game_flags(game_flag::disable_equipping);
    }

    void equip_judge::on_disable(card *target_card, player *target) {
        target->m_game->remove_game_flags(game_flag::disable_equipping);
    }
}