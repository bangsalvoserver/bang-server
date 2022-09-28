#include "characters.h"
#include "../base/requests.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_don_bell::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::post_turn_end>({target_card, 1}, [=](player *target) {
            if (p == target) {
                p->m_game->queue_action([target, target_card] {
                    target->m_game->draw_check_then(target, target_card, [target, target_card](card *drawn_card) {
                        card_suit suit = target->get_card_sign(drawn_card).suit;
                        if (suit == card_suit::diamonds || suit == card_suit::hearts) {
                            target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                            ++target->m_extra_turns;
                        }
                    });
                });
            }
        });
    }

    void effect_madam_yto::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::on_play_beer>({target_card, 1}, [=](player *target) {
            p->m_game->flash_card(target_card);
            p->draw_card(1, target_card);
        });
    }

    void effect_dutch_will::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin) {
            if (origin == target) {
                if (target->m_num_cards_to_draw > 1) {
                    target->m_game->pop_request();
                    for (int i=0; i<target->m_num_cards_to_draw; ++i) {
                        target->m_game->draw_phase_one_card_to(pocket_type::selection, target);
                    }
                    target->m_game->queue_request<request_dutch_will>(target_card, target);
                }
            }
        });
    }

    void request_dutch_will::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->add_to_hand_phase_one(target_card);
        if (target->m_game->m_selection.size() == 1) {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target->m_game->m_selection.front());
            target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
            target->add_gold(1);
            target->m_game->update_request();
        }
    }

    game_string request_dutch_will::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_DUTCH_WILL", origin_card};
        } else {
            return {"STATUS_DUTCH_WILL_OTHER", target, origin_card};
        }
    }

    void effect_josh_mccloud::on_play(card *origin_card, player *target) {
        auto *card = target->m_game->draw_shop_card();
        auto discard_drawn_card = [&]{
            target->m_game->move_card(card, pocket_type::shop_discard, nullptr, show_card_flags::pause_before_move);
        };
        if (card->color == card_color_type::black) {
            if (!target->m_game->check_flags(game_flags::disable_equipping)) {
                auto equip_set = target->make_equip_set(card);
                if (equip_set.empty()) {
                    discard_drawn_card();
                } else if (equip_set.size() == 1) {
                    equip_set.front()->equip_card(card);
                } else {
                    target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
                }
            } else {
                discard_drawn_card();
            }
        } else if (card->has_tag(tag_type::shopchoice) || target->is_possible_to_play(card)) {
            target->m_game->queue_request<request_force_play_card>(origin_card, target, card);
        } else {
            discard_drawn_card();
        }
    }
}