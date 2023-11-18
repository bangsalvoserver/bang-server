#include "lawofthewest.h"

#include "cards/base/draw.h"

#include "game/game.h"
#include "game/possible_to_play.h"

namespace banggame {
    
    void equip_lawofthewest::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [=](player *origin, card *drawn_card, shared_request_draw req_draw, bool &reveal) {
            if (req_draw->num_drawn_cards == 2) {
                reveal = true;
                event_card_key key{target_card, 1};

                if (!drawn_card->is_brown() || !drawn_card->effects.empty()) {
                    origin->m_game->add_log("LOG_MANDATORY_CARD", origin, drawn_card);
                }
                
                origin->m_game->add_listener<event_type::check_pass_turn>(key, [=](player *p, game_string &out_error) {
                    if (p == origin && drawn_card->owner == origin && is_possible_to_play(origin, drawn_card)) {
                        out_error = {"ERROR_MANDATORY_CARD", drawn_card};
                    }
                });
                origin->m_game->add_listener<event_type::on_discard_hand_card>(key, [=](player *p, card *target_card, bool used) {
                    if (p == origin && target_card == drawn_card) {
                        origin->m_game->remove_listeners(key);
                    }
                });
                origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p, bool skipped) {
                    if (p == origin) {
                        origin->m_game->remove_listeners(key);
                    }
                });
            }
        });
    }
}