#include "lawofthewest.h"

#include "cards/game_events.h"

#include "effects/base/draw.h"
#include "effects/base/pass_turn.h"

#include "game/game_table.h"
#include "game/possible_to_play.h"

namespace banggame {
    
    void equip_lawofthewest::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [=](player_ptr origin, card_ptr drawn_card, shared_request_draw req_draw, bool &reveal) {
            if (req_draw->num_drawn_cards == 2) {
                reveal = true;
                event_card_key key{target_card, 1};

                if (!drawn_card->is_brown() || !drawn_card->effects.empty()) {
                    origin->m_game->add_log("LOG_MANDATORY_CARD", origin, drawn_card);
                }
                
                origin->m_game->add_listener<event_type::check_pass_turn>(key, [=](player_ptr p, game_string &out_error) {
                    card_list modifiers;
                    if (p == origin && drawn_card->owner == origin && is_possible_to_play(origin, drawn_card, false, modifiers, {})) {
                        out_error = {"ERROR_MANDATORY_CARD", drawn_card};
                    }
                });
                origin->m_game->add_listener<event_type::on_play_card>(key, [=](player_ptr p, card_ptr target_card, const card_list &modifiers, const effect_context &ctx) {
                    if (p == origin && (target_card == drawn_card || rn::contains(modifiers, drawn_card))) {
                        origin->m_game->remove_listeners(key);
                    }
                });
                origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr p, bool skipped) {
                    if (p == origin) {
                        origin->m_game->remove_listeners(key);
                    }
                });
            }
        });
    }
}