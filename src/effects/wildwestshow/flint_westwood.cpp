#include "flint_westwood.h"

#include "effects/base/gift_card.h"
#include "effects/base/steal_destroy.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    game_string handler_flint_westwood::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target_card->owner));
        return {};
    }

    void handler_flint_westwood::on_play(card_ptr origin_card, player_ptr origin, card_ptr chosen_card, card_ptr target_card) {
        auto target = target_card->owner;
        
        for (int i=0; i<2; ++i) {
            origin->m_game->queue_action([=]{
                if (origin->alive() && target->alive() && !target->empty_hand()) {
                    card_ptr stolen_card = target->random_hand_card();

                    destroy_flags flags{ destroy_flag::intentional };
                    origin->m_game->call_event(event_type::on_destroy_card{ origin, origin_card, stolen_card, flags });
                    origin->m_game->queue_action([=]{
                        if (origin->alive() && target->alive() && stolen_card->owner == target) {
                            if (stolen_card->get_visibility() != card_visibility::shown) {
                                origin->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", origin, target, stolen_card);
                                origin->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", origin, target);
                            } else {
                                origin->m_game->add_log("LOG_STOLEN_CARD", origin, target, stolen_card);
                            }
                            origin->steal_card(stolen_card);
                        }
                    }, 42);
                }
            }, 40);
        }

        origin->m_game->queue_action([=]{
            if (origin->alive() && target->alive()) {
                handler_gift_card{false, true}.on_play(origin_card, origin, chosen_card, target);
            }
        }, 40);
    }
}