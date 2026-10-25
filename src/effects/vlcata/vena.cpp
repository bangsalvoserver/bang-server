#include "vena.h"

#include "effects/base/damage.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    void equip_vena::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>(target_card, [target, target_card](card_ptr origin_card, player_ptr origin, player_ptr hit_target, int damage, effect_flags flags) {
            if (hit_target == target && damage > 0) {
                target->m_game->queue_request<request_check>(target, target_card, [](card_sign sign) {
                    return draw_check_result{ .lucky = sign.is_hearts() || sign.rank == card_rank::rank_2 };
                }, [target, target_card, damage](bool result) {
                    if (result) {
                        target_card->flash_card();
                        target->heal(target_card, nullptr, damage);
                    }
                });
            }
        });
    }

}
