#include "invert_rotation.h"

#include "../../game.h"

namespace banggame {
    
    void effect_invert_rotation::on_enable(card *target_card, player *target) {
        target->m_game->set_game_flags(game_flags::invert_rotation);
    }
}