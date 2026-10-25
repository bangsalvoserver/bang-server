#include "bejcek.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_bejcek::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (played_card->name == "BOUNTY") {
                target_card->flash_card();
                target->draw_card(1, target_card);
            }
        });
    }

}
