#include "bomb.h"

#include "../../game.h"

namespace banggame {
    
    struct request_move_bomb : request_base {
        request_move_bomb(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, effect_flags::auto_respond) {}

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_MOVE_BOMB", origin_card};
            } else {
                return {"STATUS_MOVE_BOMB_OTHER", target, origin_card};
            }
        }
    };

    bool effect_move_bomb::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<request_move_bomb>(origin);
    }

    game_string handler_move_bomb::on_prompt(card *origin_card, player *origin, player *target) {
        if (origin == target) {
            return {"PROMPT_MOVE_BOMB_TO_SELF", origin_card};
        } else {
            return {};
        }
    }

    game_string handler_move_bomb::verify(card *origin_card, player *origin, player *target) {
        if (target != origin) {
            if (auto c = target->find_equipped_card(origin_card)) {
                return {"ERROR_DUPLICATED_CARD", c};
            }
        }
        return {};
    }

    void handler_move_bomb::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->pop_request();
        if (target != origin) {
            origin->m_game->add_log("LOG_MOVE_BOMB_ON", origin_card, origin, target);
            origin_card->on_disable(origin);
            origin_card->on_unequip(origin);
            origin_card->on_equip(target);
            target->equip_card(origin_card);
        }
        origin->m_game->update_request();
    }

    void effect_bomb::on_equip(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_discard_orange_card>(target_card, [=](player *e_target, card *e_card) {
            if (e_target == target && e_card == target_card
                && !target->m_game->is_disabled(target_card) && !target->immune_to(target_card, nullptr, {})) {
                target->m_game->add_log("LOG_CARD_EXPLODES", target_card);

                event_card_key key{target_card, 1};
                target->m_game->add_listener<event_type::on_effect_end>(key, [=](player *p, card *c) {
                    target->damage(target_card, nullptr, 2);
                    target->m_game->remove_listeners(key);
                });
            }
        });
        
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                    card_suit suit = target->get_card_sign(drawn_card).suit;
                    if (suit == card_suit::spades || suit == card_suit::clubs) {
                        target->pay_cubes(target_card, 2);
                        target->m_game->call_event<event_type::on_effect_end>(p, e_card);
                    } else {
                        target->m_game->queue_request_front<request_move_bomb>(target_card, target);
                    }
                });
            }
        });
    }

    void effect_bomb::on_unequip(card *target_card, player *target) {
        target->m_game->remove_listeners(event_card_key{target_card, 0});
    }

}