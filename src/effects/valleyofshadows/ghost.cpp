#include "ghost.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    equip_ghost::equip_ghost(int value)
        : flag(value == 1 ? player_flag::ghost_1 : player_flag::ghost_2) {}
    
    void equip_ghost::on_enable(card_ptr target_card, player_ptr target) {
        if (!target->alive()) {
            for (card_ptr c : target->m_characters) {
                target->enable_equip(c);
            }
        }
        target->add_player_flags(flag);
    }
    
    void equip_ghost::on_disable(card_ptr target_card, player_ptr target) {
        target->remove_player_flags(flag);
        if (!target->alive()) {
            target->m_game->handle_player_death(nullptr, target, discard_all_reason::discard_ghost);
        }
    }
}