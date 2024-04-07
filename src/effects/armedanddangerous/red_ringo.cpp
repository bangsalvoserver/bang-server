#include "red_ringo.h"

#include "game/game.h"

namespace banggame {
    
    void equip_red_ringo::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_game_setup>({target_card, 3}, [=](player *origin) {
            target->m_game->add_cubes(target->first_character(), max_cubes);
        });
    }
}