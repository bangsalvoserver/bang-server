#include "heal.h"

#include "game/game.h"

#include "prompts.h"

namespace banggame {
    
    game_string effect_heal::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(bot_check_target_friend(origin, target));
        MAYBE_RETURN(prompt_target_ghost(origin_card, origin, target));
        if (target->m_hp == target->m_max_hp) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else if (target->m_hp + amount > target->m_max_hp) {
            return {"PROMPT_WASTEFUL_HEAL", origin_card, amount, target->m_hp + amount - target->m_max_hp};
        }
        return {};
    }

    void effect_heal::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        if (!target->immune_to(origin_card, origin, flags)) {
            target->heal(amount);
        }
    }

    game_string effect_heal_notfull::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (target->m_hp == target->m_max_hp) {
            return "ERROR_CANT_HEAL_PAST_FULL_HP";
        }
        return {};
    }
}