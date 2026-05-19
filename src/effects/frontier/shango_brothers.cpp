#include "shango_brothers.h"

#include "effects/wildwestshow/ruleset.h"
#include "effects/base/draw.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_shango_brothers::on_enable(card_ptr target_card, player_ptr target) {
        auto played_count = std::make_shared<int>();

        target->m_game->add_listener<event_type::on_turn_start>({target_card, 10}, [=](player_ptr origin) {
            if (origin == target) {
                *played_count = 0;
            }
        });

        target->m_game->add_listener<event_type::on_play_card>(target_card, [=, ncards=ncards](player_ptr origin, card_ptr origin_card, const card_list &modifiers, const effect_context &ctx) {
            if (origin == target && origin == origin->m_game->m_playing) {
                *played_count += count_played_cards(origin_card, modifiers, ctx);

                int cards_to_draw = *played_count / ncards;
                *played_count %= ncards;
                
                if (cards_to_draw != 0) {
                    target->m_game->queue_action([=]{
                        target->m_game->queue_request<request_can_draw>(target_card, nullptr, target, cards_to_draw);
                    }, -50);
                }
            }
        });
    }
}