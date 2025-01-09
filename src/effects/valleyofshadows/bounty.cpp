#include "bounty.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "effects/base/damage.h"

namespace banggame {

    game_string equip_bounty::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }
    
    void equip_bounty::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 4}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && target == p && flags.check(effect_flag::is_bang)) {
                target_card->flash_card();
                origin->draw_card(1, target_card);
            }
        });
    }
}