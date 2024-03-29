#include "thedoctor.h"

#include "game/game.h"

namespace banggame {

    void equip_thedoctor::on_enable(card *target_card, player *target) {
        int min_hp = rn::min(target->m_game->m_players
            | rv::filter(&player::alive)
            | rv::transform(&player::m_hp));
        
        for (player *p : range_all_players(target)) {
            if (p->m_hp == min_hp) {
                p->heal(1);
            }
        }
    }
}