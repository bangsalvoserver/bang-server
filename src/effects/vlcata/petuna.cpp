#include "petuna.h"
#include "cards/game_events.h"
#include "game/game_table.h"
#include "effects/base/draw_check.h"

namespace banggame {
    void equip_petuna::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>({target_card, 1},
            [target, target_card](player_ptr origin, bool skipped) {
                if (origin == target && !skipped && !target->get_custom_flag("petuna_extra_turn")) {
                    target->m_game->queue_request<request_draw_check>(
                        target_card, target,
                        [target, target_card](card_sign sign) {
                            if (sign.is_spades()) {
                                target->damage(target_card, target, 1);
                            } else if (sign.is_red()) {
                                target->set_custom_flag("petuna_extra_turn", true);
                                target->m_game->queue_action([target]{
                                    target->m_game->start_turn(target);
                                });
                            }
                        });
                }
            });
        
        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [target](player_ptr origin) {
                if (origin != target) {
                    target->set_custom_flag("petuna_extra_turn", false);
                }
            });
    }
}