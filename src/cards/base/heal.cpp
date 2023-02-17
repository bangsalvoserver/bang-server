#include "heal.h"

#include "game/game.h"
#include "prompts.h"

namespace banggame {
    
    game_string effect_heal::on_prompt(card *origin_card, player *origin, player *target) {
        MAYBE_RETURN(prompt_target_ghost{}.on_prompt(origin_card, origin, target));
        if (target->m_hp == target->m_max_hp) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
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

    game_string handler_heal_multi::on_prompt(card *origin_card, player *origin, int amount) {
        return effect_heal(amount).on_prompt(origin_card, origin);
    }

    void handler_heal_multi::on_play(card *origin_card, player *origin, int amount) {
        effect_heal(amount).on_play(origin_card, origin);
    }
}