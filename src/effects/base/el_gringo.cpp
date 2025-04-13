#include "el_gringo.h"

#include "game/game_table.h"
#include "effects/base/damage.h"

namespace banggame {
    
    void equip_el_gringo::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 2}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && p == target) {
                origin->m_game->queue_action([=]{
                    if (target->alive() && p->m_game->m_playing != p && !origin->empty_hand()) {
                        target_card->flash_card();
                        for (int i=0; i<damage && !origin->empty_hand(); ++i) {
                            card_ptr stolen_card = origin->random_hand_card();
                            if (stolen_card->get_visibility() != card_visibility::shown) {
                                target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", target, origin, stolen_card);
                                target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", target, origin);
                            } else {
                                target->m_game->add_log("LOG_STOLEN_CARD", target, origin, stolen_card);
                            }
                            target->steal_card(stolen_card);
                        }
                    }
                });
            }
        });
    }
}