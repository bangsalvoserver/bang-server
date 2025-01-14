#include "ghosttown.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

#include "effects/base/draw.h"
#include "effects/base/death.h"

namespace banggame {

    void equip_ghosttown::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::check_revivers>(target_card, [=](player_ptr target) {
            if (!target->alive()) {
                origin->m_game->add_log("LOG_REVIVE", target, target_card);
                target->add_player_flags(player_flag::temp_ghost);
                target->enable_equip(target->get_character());
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>({target_card, -3}, [](player_ptr target, bool skipped) {
            target->m_game->queue_action([=]{
                if (target->m_extra_turns == 0 && target->remove_player_flags(player_flag::temp_ghost) && !target->alive()) {
                    handle_player_death(nullptr, target, death_type::ghosttown_turn_end);
                }
            }, -10);
        });
        origin->m_game->add_listener<event_type::count_cards_to_draw>({target_card, 2}, [](player_ptr target, int &value) {
            if (target->check_player_flags(player_flag::temp_ghost) && value == 2) {
                value = 3;
            }
        });
    }
}