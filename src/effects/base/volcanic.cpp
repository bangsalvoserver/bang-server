#include "volcanic.h"

#include "game/game_table.h"

#include "bang.h"

namespace banggame {

    int equip_volcanic::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        // TODO: the bot should not play volcanic if his character is Willy the Kid
        // also, replacing the current weapon card should be considered in the scoring
        if (target->get_bangs_played() <= 0) {
            return -20;
        }
        return 0;
    }
    
    void equip_volcanic::on_enable(card_ptr target_card, player_ptr target) const {
        target->m_game->add_listener<event_type::count_bangs_played>(target_card, [=](const_player_ptr origin, int &value, bool real_count) {
            if (origin == target && !real_count) {
                value = 0;
            }
        });
    }

    void equip_volcanic::on_disable(card_ptr target_card, player_ptr target) const {
        target->m_game->remove_listeners(event_card_key{target_card, 0});
    }
}
