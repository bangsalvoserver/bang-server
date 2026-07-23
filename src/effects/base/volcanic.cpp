#include "volcanic.h"

#include "game/game_table.h"

#include "bang.h"

namespace banggame {

    prompt_string equip_volcanic::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin->is_bot() && target->get_bangs_played() <= 0) {
            return {1, "BOT_NO_BANGS_PLAYED"};
        }
        return {};
    }
    
    void equip_volcanic::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_bangs_played>(target_card, [=](const_player_ptr origin, int &value, bool real_count) {
            if (origin == target && !real_count) {
                value = 0;
            }
        });
    }

    void equip_volcanic::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(event_card_key{target_card, 0});
    }
}