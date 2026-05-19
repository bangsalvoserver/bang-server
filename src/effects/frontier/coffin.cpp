#include "coffin.h"

#include "effects/base/utils.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    namespace event_type {
        struct check_coffin {
            using result_type = bool;
            card_ptr target_card;
            player_ptr target;
        };
    }
    
    void equip_coffin::on_enable(card_ptr target_card, player_ptr target) {
        if (target->m_game->call_event(event_type::check_coffin{ target_card, target })) {
            equip_add_flag{player_flag::keep_alive}.on_enable(target_card, target);
            target->add_player_flags(player_flag::coffin);
        }

        target->m_game->add_listener<event_type::on_turn_end>({target_card, -10}, [=](player_ptr origin, bool skipped) {
            if (origin == target) {
                equip_add_flag{player_flag::keep_alive}.on_enable(target_card, target);
                target->add_player_flags(player_flag::coffin);

                target->m_game->add_listener<event_type::check_coffin>({target_card, 50}, [=](card_ptr e_target_card, player_ptr e_target) {
                    return e_target_card == target_card && e_target == target;
                });
            }
        });
        
        target->m_game->add_listener<event_type::check_revivers>({target_card, 2}, [=](player_ptr origin) {
            if (origin == target && target->check_player_flags(player_flag::coffin)) {
                origin->discard_card(target_card);
            }
        });
    }

    void equip_coffin::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners({ target_card, -10 });
        target->m_game->remove_listeners({ target_card, 2 });

        target->remove_player_flags(player_flag::coffin);
        equip_add_flag{player_flag::keep_alive}.on_disable(target_card, target);
    }

    void equip_coffin_nodisable::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners({ target_card, 50 });
    }
}