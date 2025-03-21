#include "ghost.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "effects/base/death.h"

namespace banggame {

    game_string equip_ghost::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_friend(origin, target));
        return {};
    }
    
    void equip_ghost::on_enable(card_ptr target_card, player_ptr target) {
        if (!target->alive()) {
            target->enable_equip(target->get_character());
        }
    }
    
    void equip_ghost::on_disable(card_ptr target_card, player_ptr target) {
        if (!target->alive()) {
            handle_player_death(nullptr, target, death_type::ghost_discard);
        }
    }
}