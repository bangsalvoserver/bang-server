#include "tuctuc.h"
#include "cards/game_events.h"
#include "effects/base/heal.h"
#include "game/game_table.h"

namespace banggame {
    void equip_tuctuc::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_heal>(target_card,
            [=](card_ptr origin_card, player_ptr origin, player_ptr healed_player, int value) {
                if (healed_player == target && value > 0) {
                    target->m_game->queue_action([=]{
                        if (target->alive()) {
                            target_card->flash_card();
                            target->draw_card(2, target_card);
                        }
                    });
                }
            });
    }
}