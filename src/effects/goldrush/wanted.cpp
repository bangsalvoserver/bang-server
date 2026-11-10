#include "wanted.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

#include "effects/base/death.h"

namespace banggame {

    game_string equip_wanted::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        if (target->is_sheriff()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    int equip_wanted::evaluate(card_ptr, player_ptr origin, player_ptr target) const {
        if (bot_suggestion::is_target_enemy(origin, target, bot_suggestion::action_type::jail)) return 5;
        return bot_suggestion::is_target_friend(origin, target) ? -5 : -1;
    }

    void equip_wanted::on_enable(card_ptr target_card, player_ptr p) const {
        p->m_game->add_listener<event_type::on_player_eliminated>({target_card, 3}, [p, target_card](player_ptr origin, player_ptr target, death_type type) {
            if (type == death_type::death && origin && origin->alive() && p == target && origin != target) {
                target_card->flash_card();
                origin->draw_card(2, target_card);
                origin->add_gold(1);
            }
        });
    }
}
