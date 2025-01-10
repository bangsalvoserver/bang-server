#include "molly_stark.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_molly_stark::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_discard_hand_card>(origin_card, [=](player_ptr target, card_ptr target_card, bool used) {
            if (origin == target && used && origin->m_game->m_playing != origin) {
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {                        
                        origin_card->flash_card();
                        origin->draw_card(1, origin_card);
                    }
                }, -1);
            }
        });
    }
}