#include "julie_cutter.h"

#include "game/game_table.h"
#include "effects/base/bang.h"
#include "effects/base/damage.h"
#include "effects/base/draw_check.h"

namespace banggame {
    
    void equip_julie_cutter::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && p == target && origin != target) {
                origin->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_red, [=](bool result) {
                            if (result) {
                                target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                                target->m_game->queue_request<request_bang>(target_card, target, origin);
                            }
                        });
                    }
                });
            }
        });
    }
}