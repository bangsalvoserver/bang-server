#include "red_ringo.h"

#include "game/game.h"

namespace banggame {
    
    void equip_red_ringo::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_game_setup>({target_card, 3}, [=](player_ptr origin) {
            target->first_character()->add_cubes(max_cubes);
        });
    }
}