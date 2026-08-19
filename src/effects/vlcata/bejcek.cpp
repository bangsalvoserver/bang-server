#include "bejcek.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_bejcek_passive::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card,
            [target, target_card](player_ptr origin, card_ptr played_card) {
                if (played_card->name == "BOUNTY" && target->alive()) {
                    target_card->flash_card();
                    target->draw_card(1, target_card);
                }
            });
    }
}