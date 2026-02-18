#include "suit_modifier.h"

#include "game/game_table.h"

#include "effects/base/draw_check.h"

namespace banggame {
    
    void equip_suit_modifier::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_suit_modifier>(target_card, [suit=suit]{
            return suit;
        });
    }
}