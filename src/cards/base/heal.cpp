#include "heal.h"

#include "game/game.h"
#include "prompts.h"

namespace banggame {
    
    game_string effect_heal::on_prompt(card *origin_card, player *origin, player *target) {
        MAYBE_RETURN(prompt_target_ghost{}.on_prompt(origin_card, origin, target));
        if (origin->m_hp + amount > origin->m_max_hp) {
            return {"PROMPT_WASTEFUL_HEAL", origin_card, amount, origin->m_hp + amount - origin->m_max_hp};
        }
        return {};
    }

    void effect_heal::on_play(card *origin_card, player *origin, player *target) {
        target->heal(amount);
    }

    game_string effect_heal_notfull::get_error(card *origin_card, player *origin, player *target) {
        if (target->m_hp == target->m_max_hp) {
            return "ERROR_CANT_HEAL_PAST_FULL_HP";
        }
        return {};
    }

    void effect_queue_heal_notfull::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->queue_action([target, amount=amount]{
            target->heal(amount);
        });
    }
}