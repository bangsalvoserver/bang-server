#include "deadman.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

#include "ruleset.h"

namespace banggame {

    void equip_deadman::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::check_revivers>(target_card, [=](player_ptr target) {
            player_ptr first_dead = nullptr;
            origin->m_game->call_event(event_type::get_first_dead_player{ first_dead });

            if (!target->alive() && target == first_dead) {
                origin->m_game->remove_listeners(target_card);
                
                target_card->flash_card();
                origin->m_game->add_log("LOG_REVIVE", target, target_card);

                target->remove_player_flags(player_flag::dead);
                target->set_hp(2);
                target->draw_card(2);

                for (card_ptr c : target->m_characters) {
                    target->enable_equip(c);
                }
            }
        });
    }
}