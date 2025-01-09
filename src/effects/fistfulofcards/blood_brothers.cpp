#include "blood_brothers.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "effects/base/can_play_card.h"

namespace banggame {

    void equip_blood_brothers::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>({target_card, 2}, [=](player_ptr origin) {
            target->m_game->queue_request<request_can_play_card>(target_card, nullptr, origin);
        });
    }

    game_string effect_blood_brothers::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin->m_hp <= 1) {
            return "ERROR_CANT_SELF_DAMAGE";
        }
        if (target->m_hp == target->m_max_hp) {
            return {"ERROR_PLAYER_IS_FULL_HP", target};
        }
        return {};
    }

    game_string effect_blood_brothers::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_friend(origin, target));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }

    void effect_blood_brothers::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->damage(origin_card, origin, 1);
        origin->m_game->queue_action([=]{ target->heal(1); });
    }

}