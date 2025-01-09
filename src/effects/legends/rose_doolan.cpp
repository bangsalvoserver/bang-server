#include "rose_doolan.h"

#include "game/game_table.h"

#include "cards/game_enums.h"

namespace banggame {

    void equip_rose_doolan_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->add_player_flags(player_flag::ignore_distances);
    }

    void equip_rose_doolan_legend::on_disable(card_ptr target_card, player_ptr target) {
        target->remove_player_flags(player_flag::ignore_distances);
    }
}