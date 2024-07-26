#include "wildwestshow.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_wildwestshow::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_game_flags(game_flag::free_for_all);
    }

    void equip_wildwestshow::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_game_flags(game_flag::free_for_all);
    }
}