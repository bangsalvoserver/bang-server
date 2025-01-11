#include "brothel.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "cards/game_events.h"

#include "effects/base/death.h"
#include "effects/base/predraw_check.h"

namespace banggame {

    static uint8_t brothel_counter = 0;

    game_string equip_brothel::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }

    void equip_brothel::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_predraw_check>({target_card, 0}, [=](player_ptr p, card_ptr e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->queue_request<request_check>(target, target_card, &card_sign::is_red, [=](bool result) {
                    target->discard_card(target_card);
                    if (!result) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        event_card_key event_key{target_card, 1 + brothel_counter++ % 20};
                        target->m_game->add_disabler(event_key, [=](const_card_ptr c) {
                            return c->pocket == pocket_type::player_character && c->owner == target;
                        });
                        auto clear_events = [target, event_key](player_ptr p) {
                            if (p == target) {
                                target->m_game->remove_disablers(event_key);
                                target->m_game->remove_listeners(event_key);
                            }
                        };
                        target->m_game->add_listener<event_type::pre_turn_start>(event_key, clear_events);
                        target->m_game->add_listener<event_type::on_player_eliminated>(event_key, [=](player_ptr killer, player_ptr p, death_type type) {
                            clear_events(p);
                        });
                    }
                });
            }
        });
    }

    void equip_brothel::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners(event_card_key{target_card, 0});
    }
}