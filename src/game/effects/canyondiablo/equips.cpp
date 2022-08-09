#include "equips.h"

#include "../../game.h"

namespace banggame {

    using namespace enums::flag_operators;

    void effect_packmule::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::apply_maxcards_modifier>(target_card, [p](player *origin, int &value) {
            if (origin == p) {
                ++value;
            }
        });
    }

    void effect_indianguide::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::apply_immunity_modifier>(target_card, [p](card *origin_card, player *e_origin, const player *e_target, effect_flags flags, bool &value) {
            if (e_target == p && origin_card->has_tag(tag_type::indians)) {
                value = true;
            }
        });
    }

    void effect_taxman::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card && !target->m_game->check_flags(game_flags::phase_one_override)) {
                target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                    auto suit = target->get_card_sign(drawn_card).suit;
                    if (suit == card_suit::clubs || suit == card_suit::spades) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        --target->m_num_cards_to_draw;
                        event_card_key key{target_card, 1};
                        target->m_game->add_listener<event_type::post_draw_cards>(key, [=](player *origin) {
                            if (origin == target) {
                                ++target->m_num_cards_to_draw;
                                origin->m_game->remove_listeners(key);
                            }
                        });
                    }
                });
            }
        });
    }

    void effect_brothel::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_predraw_check>(target_card, [=](player *p, card *e_card) {
            if (p == target && e_card == target_card) {
                target->m_game->draw_check_then(target, target_card, [=](card *drawn_card) {
                    target->discard_card(target_card);
                    auto suit = target->get_card_sign(drawn_card).suit;
                    if (suit == card_suit::clubs || suit == card_suit::spades) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        event_card_key event_key{target_card, 1 + effect_holder_counter++ % 20};
                        target->m_game->add_disabler(event_key, [=](card *c) {
                            return c->pocket == pocket_type::player_character && c->owner == target;
                        });
                        auto clear_events = [target, event_key](player *p) {
                            if (p == target) {
                                target->m_game->remove_disablers(event_key);
                                target->m_game->remove_listeners(event_key);
                            }
                        };
                        target->m_game->add_listener<event_type::pre_turn_start>(event_key, clear_events);
                        target->m_game->add_listener<event_type::on_player_death>(event_key, [=](player *killer, player *p) {
                            clear_events(p);
                        });
                    }
                });
            }
        });
    }

    static bool is_discard_bronco(card *c) {
        return c->has_tag(tag_type::bronco);
    }

    void effect_bronco::on_equip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_hidden_deck, is_discard_bronco);
        if (it != target->m_game->m_hidden_deck.end()) {
            card *found_card = *it;
            target->m_game->move_card(found_card, pocket_type::specials, nullptr, show_card_flags::instant | show_card_flags::hidden);
            target->m_game->send_card_update(found_card, nullptr, show_card_flags::instant);
        }
    }

    void effect_bronco::on_unequip(card *origin_card, player *target) {
        auto it = std::ranges::find_if(target->m_game->m_specials, is_discard_bronco);
        if (it != target->m_game->m_specials.end()) {
            target->m_game->move_card(*it, pocket_type::hidden_deck, nullptr, show_card_flags::instant | show_card_flags::hidden);
        }
    }
}