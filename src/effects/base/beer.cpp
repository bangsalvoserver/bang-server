#include "beer.h"

#include "game/game.h"

namespace banggame {

    static int get_beer_heal_amount(player_ptr target) {
        int amount = 1;
        target->m_game->call_event(event_type::apply_beer_modifier{ target, amount });
        return amount;
    }
    
    game_string effect_beer::on_prompt(card_ptr origin_card, player_ptr target) {
        if (target->m_game->num_alive() <= 2 || (target->m_hp == target->m_max_hp)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        int amount = get_beer_heal_amount(target);
        if (target->m_hp + amount > target->m_max_hp) {
            return {"PROMPT_WASTEFUL_HEAL", origin_card, amount, target->m_hp + amount - target->m_max_hp};
        }
        return {};
    }

    void effect_beer::on_play(card_ptr origin_card, player_ptr target) {
        if (target->m_game->num_alive() > 2) {
            target->heal(get_beer_heal_amount(target));
        }
        target->m_game->call_event(event_type::on_play_beer{ target });
    }

    bool effect_beer::can_play(card_ptr origin_card, player_ptr target) {
        return !target->m_game->pending_requests() || target->m_game->num_alive() > 2;
    }
}