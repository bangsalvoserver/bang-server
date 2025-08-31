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
        if (disable_character) {
            target->m_game->add_disabler(target_card, [=](const_card_ptr c) {
                return c == target->get_character();
            });
        }

        if (!target->alive()) {
            target->enable_equip(target->get_character());
        }

        target->add_player_flags(flag);
    }
    
    void equip_ghost::on_disable(card_ptr target_card, player_ptr target) {
        target->remove_player_flags(flag);
        
        if (!target->alive()) {
            handle_player_death(nullptr, target, death_type::ghost_discard);
        }

        if (disable_character) {
            target->m_game->queue_action([=]{
                target->m_game->remove_disablers(target_card);
            }, 49);
        }
    }
}