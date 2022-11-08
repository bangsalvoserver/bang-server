#include "lawofthewest.h"

#include "game/game.h"

namespace banggame {
    
    void effect_lawofthewest::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [=](player *origin, card *drawn_card, bool &reveal) {
            if (origin->m_num_drawn_cards == 2) {
                reveal = true;
                event_card_key key{target_card, 1};

                if (drawn_card->color != card_color_type::brown || !drawn_card->effects.empty()) {
                    origin->m_game->add_listener<event_type::post_draw_cards>(key, [=](player *p) {
                        if (p == origin) {
                            origin->m_game->add_log("LOG_MANDATORY_CARD", origin, drawn_card);
                        }
                    });
                }
                
                if (drawn_card->color == card_color_type::brown) {
                    origin->m_game->add_listener<event_type::verify_pass_turn>(key, [=](player *p, game_string &out_error) {
                        if (p == origin && drawn_card->owner == origin && origin->is_possible_to_play(drawn_card)) {
                            out_error = {"ERROR_MANDATORY_CARD", drawn_card};
                        }
                    });
                } else {
                    origin->m_game->add_listener<event_type::verify_pass_turn>(key, [=](player *p, game_string &out_error) {
                        if (p == origin && drawn_card->owner == origin && !origin->make_equip_set(drawn_card).empty()) {
                            out_error = {"ERROR_MANDATORY_CARD", drawn_card};
                        }
                    });
                }
                origin->m_game->add_listener<event_type::on_effect_end>(key, [=](player *p, card *played_card) {
                    if (p == origin && played_card == drawn_card) {
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