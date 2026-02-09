#include "luckycharm.h"

#include "game/game_table.h"
#include "effects/base/damage.h"

namespace banggame {

    void equip_luckycharm::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (p == target) {
                target->m_game->queue_action([=]{
                    if (target->alive()) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        target_card->flash_card();
                        target->add_gold(damage);
                    }
                });
            }
        });
    }
}