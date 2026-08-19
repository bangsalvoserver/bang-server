#include "terka_p.h"
#include "cards/game_events.h"
#include "game/game_table.h"
#include "effects/base/bang.h"

namespace banggame {
    void equip_terka_p_bang::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_bang_modifier>(target_card,
            [target, target_card](player_ptr origin, shared_request_bang req) {
                if (origin == target && !target->get_custom_flag("terka_banged")) {
                    target->set_custom_flag("terka_banged", true);
                    req->flags.add(effect_flag::heal_choice);
                }
            });
        
        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [target](player_ptr origin) {
                if (origin == target) target->set_custom_flag("terka_banged", false);
            });
    }
}