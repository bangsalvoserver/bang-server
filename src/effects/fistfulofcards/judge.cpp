#include "judge.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    void equip_judge::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_game_flags(game_flag::disable_equipping);
    }

    void equip_judge::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_game_flags(game_flag::disable_equipping);
    }
}