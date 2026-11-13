#include "ghost.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

#include "effects/base/death.h"
#include "effects/base/utils.h"

namespace banggame {

    void equip_disable_character::on_enable(card_ptr target_card, player_ptr target) const {
        target->m_game->add_disabler(target_card, [=](const_card_ptr c) {
            return c->pocket == pocket_type::player_character && c->owner == target;
        });
    }

    void equip_disable_character::on_disable(card_ptr target_card, player_ptr target) const {
        target->m_game->queue_action([=]{
            target->m_game->remove_disabler(target_card);
        }, 49);
    }

    int equip_ghost::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return bot_suggestion::evaluate_help(origin, target);
    }
    
    void equip_ghost::on_enable(card_ptr target_card, player_ptr target) const {
        if (!target->in_game()) {
            revive_character(target);
        }

        equip_add_flag{player_flag::keep_alive}.on_enable(target_card, target);
    }
    
    void equip_ghost::on_disable(card_ptr target_card, player_ptr target) const {
        equip_add_flag{player_flag::keep_alive}.on_disable(target_card, target);

        if (!target->in_game()) {
            handle_player_death(nullptr, target, death_type::ghost_discard);
        }
    }
}
