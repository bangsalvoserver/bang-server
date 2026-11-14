#include "martin.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_martin::on_enable(card_ptr target_card, player_ptr target) {
        auto played_bang = std::make_shared<bool>(false);

        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [played_bang, target](player_ptr origin) {
                if (origin == target) {
                    *played_bang = false;
                }
            });

        target->m_game->add_listener<event_type::on_play_card>(target_card,
            [played_bang, target](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
                if (origin == target && played_card->is_bang_card(target)) {
                    *played_bang = true;
                }
            });

        target->m_game->add_listener<event_type::on_turn_end>(target_card,
            [played_bang, target, target_card](player_ptr origin, bool skipped) {
                if (origin == target && !*played_bang && !skipped) {
                    target_card->flash_card();
                    target->draw_card(1, target_card);
                }
            });
    }
}