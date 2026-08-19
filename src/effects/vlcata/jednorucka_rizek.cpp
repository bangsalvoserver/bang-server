#include "jednorucka_rizek.h"
#include "cards/game_events.h"
#include "game/game_table.h"

namespace banggame {
    void equip_jednorucka_rizek::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_death_immunity>({target_card, 5},
            [target, target_card](player_ptr victim, bool &immune) {
                if (victim == target && !target->get_custom_flag("rizek_used")) {
                    target->set_custom_flag("rizek_used", true);
                    immune = true;
                    target_card->flash_card();
                    target->set_hp(3);
                    target->draw_card(3, target_card);
                }
            });
    }
}