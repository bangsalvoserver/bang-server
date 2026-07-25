#include "coffee.h"

#include "game/game_table.h"

#include "cards/game_events.h"

namespace banggame {

    game_string effect_coffee::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->is_bot() && origin->m_hand.size() <= origin->max_cards_end_of_turn()) {
            return "BOT_BAD_COFFEE";
        }
        return {};
    }
    
    void effect_coffee::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::apply_maxcards_modifier>({ origin_card, -10}, [=](const_player_ptr target, int &value) {
            if (target == origin) {
                value = target->m_hand.size();
            }
        });

        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player_ptr target, bool skipped) {
            if (target == origin) {
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }

}