#include "coffin.h"

#include "game/game_table.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

namespace banggame {
    
    void equip_coffin::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, -10}, [=](player_ptr origin, bool skipped) {
            if (origin == target) {
                target->add_player_flags(player_flag::coffin);

                target->m_game->add_listener<event_type::check_revivers>({target_card, 2}, [=](player_ptr e_origin) {
                    if (origin == e_origin) {
                        origin->discard_card(target_card);
                    }
                });
            }
        });
    }

    void equip_coffin::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(target_card);
        target->remove_player_flags(player_flag::coffin);
    }
}