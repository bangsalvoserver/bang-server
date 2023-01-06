#include "invert_rotation.h"

#include "game/game.h"

namespace banggame {
    
    void equip_invert_rotation::on_enable(card *target_card, player *target) {
        target->m_game->add_game_flags(game_flags::invert_rotation);
    }

    void equip_invert_rotation::on_disable(card *target_card, player *target) {
        target->m_game->remove_game_flags(game_flags::invert_rotation);
    }
}