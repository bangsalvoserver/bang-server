#include "heal.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

namespace banggame {
    
    prompt_string effect_heal::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) const {
        MAYBE_RETURN(prompts::prompt_target_immunity(origin_card, origin, target, flags));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        if (target->m_hp == target->m_max_hp) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else if (target->m_hp + amount > target->m_max_hp) {
            return {"PROMPT_WASTEFUL_HEAL", origin_card, amount, target->m_hp + amount - target->m_max_hp};
        }
        return {};
    }

    int effect_heal::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        if (bot_suggestion::is_target_friend(origin, target)) {
            int score = 4;
            if (target->m_hp <= 2) {
                if (target->is_sheriff()) {
                    score += 10;
                } else {
                    ++score;
                }
            }
            return score;
        } else if (bot_suggestion::is_target_enemy(origin, target)) {
            return -5;
        } else if (target->is_sheriff() && target->m_hp <= 1) {
            // Renegade will try to save the sheriff
            return 2;
        } else {
            return -2;
        }
    }

    void effect_heal::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) const {
        bot_suggestion::signal_helpful_action(origin, target, flags);

        if (!target->immune_to(origin_card, origin, flags)) {
            target->heal(origin_card, origin, amount);
        }
    }

    game_string effect_heal_notfull::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        if (target->m_hp == target->m_max_hp) {
            return "ERROR_CANT_HEAL_PAST_FULL_HP";
        }
        return {};
    }
}
