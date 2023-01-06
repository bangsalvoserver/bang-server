#include "abandonedmine.h"

#include "game/game.h"

namespace banggame {
    
    void equip_abandonedmine::on_enable(card *target_card, player *target) {
        target->m_game->add_game_flags(game_flags::phase_one_draw_discard);
    }

    void equip_abandonedmine::on_disable(card *target_card, player *target) {
        target->m_game->remove_game_flags(game_flags::phase_one_draw_discard);
    }
}