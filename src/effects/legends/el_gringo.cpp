#include "el_gringo.h"

#include "effects/base/damage.h"
#include "effects/base/steal_destroy.h"

#include "game/game_table.h"

namespace banggame {

    void equip_el_gringo_legend::on_enable(card_ptr target_card, player_ptr p) {
        p->m_game->add_listener<event_type::on_hit>({target_card, 2}, [=](card_ptr origin_card, player_ptr origin, player_ptr target, int damage, effect_flags flags) {
            if (origin && p == target && origin != target) {
                origin->m_game->queue_action([=]{
                    if (target->alive()) {
                        bool flashed = false;
                        if (!origin->empty_hand()) {
                            target_card->flash_card();
                            flashed = true;

                            for (int i=0; i<damage && !origin->empty_hand(); ++i) {
                                card_ptr stolen_card = origin->random_hand_card();
                                if (stolen_card->get_visibility() != card_visibility::shown) {
                                    target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", target, origin, stolen_card);
                                    target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", target, origin);
                                } else {
                                    target->m_game->add_log("LOG_STOLEN_CARD", target, origin, stolen_card);
                                }
                            
                                destroy_flags flags{};
                                target->m_game->call_event(event_type::on_destroy_card{ target, target_card, stolen_card, flags });
                                target->steal_card(stolen_card);
                            }
                        }

                        if (origin_card && origin_card->deck == card_deck_type::main_deck && origin_card->owner != target) {
                            if (!flashed) {
                                target_card->flash_card();
                            }

                            target->m_game->add_log("LOG_STOLEN_SELF_CARD", target, origin_card);
                            origin_card->add_short_pause();
                            target->add_to_hand(origin_card);
                        }
                    }
                });
            }
        });
    }
}