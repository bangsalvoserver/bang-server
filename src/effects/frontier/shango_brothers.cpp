#include "shango_brothers.h"

#include "effects/wildwestshow/ruleset.h"
#include "effects/base/draw.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_shango_brothers::on_enable(card_ptr target_card, player_ptr target) {
        struct shango_brothers_state {
            int played_count = 0;
            int cards_drawn = 0;
        };
        
        auto state = std::make_shared<shango_brothers_state>();

        target->m_game->add_listener<event_type::on_turn_start>({target_card, 10}, [=](player_ptr origin) {
            if (origin == target) {
                *state = {};
            }
        });

        target->m_game->add_listener<event_type::on_play_card>(target_card, [=, card_count=card_count, max_cards=max_cards](player_ptr origin, card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (origin == target && origin == origin->m_game->m_playing && state->cards_drawn < max_cards) {
                state->played_count += count_played_cards(origin_card, modifiers, ctx);

                int cards_to_draw = state->played_count / card_count;
                state->played_count %= card_count;
                
                state->cards_drawn += cards_to_draw;
                if (state->cards_drawn > max_cards) {
                    cards_to_draw -= (state->cards_drawn - max_cards);
                }
                if (cards_to_draw != 0) {
                    target->m_game->queue_action([=]{
                        if (target->alive()) {
                            target->draw_card(cards_to_draw, target_card);
                        }
                    }, -2);
                }
            }
        });
    }
}