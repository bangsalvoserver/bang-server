#include "thedoctor.h"

#include "game/game.h"

namespace banggame {

    void equip_thedoctor::on_enable(card *target_card, player *target) {
        int min_hp = std::ranges::min(target->m_game->m_players
            | std::views::filter(&player::alive)
            | std::views::transform(&player::m_hp));
        
        for (player &p : range_all_players(target)) {
            if (p.m_hp == min_hp) {
                p.heal(1);
            }
        }
    }
}