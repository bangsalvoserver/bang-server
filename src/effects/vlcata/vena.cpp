#include "vena.h"
#include "cards/game_events.h"
#include "game/game_table.h"
#include "effects/base/draw_check.h"

namespace banggame {
    void equip_vena::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::check_damage_response>(target_card,
            [target, target_card](player_ptr origin, player_ptr victim, int &damage) {
                if (victim == target && damage > 0) {
                    target->m_game->queue_request<request_draw_check>(
                        target_card, target,
                        [target, &damage](card_sign sign) {
                            if (sign.is_hearts() || sign.rank == 2) {
                                target->m_game->add_log("LOG_VENA_SAVED", target);
                                --damage;
                            }
                        });
                }
            });
    }
}