#include "horous.h"
#include "cards/game_events.h"
#include "effects/base/draw_check.h"
#include "effects/base/bang.h"
#include "game/game_table.h"

namespace banggame {
    void equip_horous::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card,
            [target, target_card](player_ptr origin, bool skipped) {
                if (origin == target && !skipped) {
                    target->m_game->queue_request<request_check>(
                        target, target_card,
                        [](card_sign sign) { return sign.is_spades() || sign.is_clubs(); },
                        [target, target_card](bool result) {
                            if (result) {
                                target->m_game->add_log("LOG_HOROUS_SHADOW", target);
                                target->m_game->add_listener<event_type::check_bang_target>({target_card, 10},
                                    [target](card_ptr origin_card, player_ptr origin, player_ptr target_player, effect_flags flags) -> game_string {
                                        if (target_player == target) {
                                            return "ERROR_TARGET_IMMUNE";
                                        }
                                        return {};
                                    });
                                target->m_game->add_listener<event_type::on_turn_start>({target_card, 10},
                                    [target, target_card](player_ptr p) {
                                        if (p == target) {
                                            target->m_game->remove_listeners(target_card);
                                        }
                                    });
                            }
                        });
                }
            });
    }
}