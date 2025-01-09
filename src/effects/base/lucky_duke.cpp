#include "lucky_duke.h"

#include "draw_check.h"

#include "game/game_table.h"

namespace banggame {
    
    void equip_lucky_duke::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_num_checks>(target_card, [=](const_player_ptr origin, int &num_checks) {
            if (origin == target) {
                ++num_checks;
            }
        });
    }
}