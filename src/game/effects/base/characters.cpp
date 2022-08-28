#include "characters.h"
#include "requests.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_calamity_janet::on_enable(card *origin_card, player *p) {
        p->add_player_flags(player_flags::treat_missed_as_bang);
    }

    void effect_calamity_janet::on_disable(card *origin_card, player *p) {
        p->remove_player_flags(player_flags::treat_missed_as_bang);
    }

    void effect_slab_the_killer::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(target_card, [p](player *target, request_bang *req) {
            if (p == target) {
                ++req->bang_strength;
            }
        });
    }

    void effect_black_jack::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_card_drawn>(target_card, [target, target_card](player *origin, card *drawn_card, bool &reveal) {
            if (origin == target && origin->m_num_drawn_cards == 2) {
                reveal = true;

                card_suit suit = target->get_card_sign(drawn_card).suit;
                if (suit == card_suit::hearts || suit == card_suit::diamonds) {
                    event_card_key key{target_card, 2};
                    origin->m_game->add_listener<event_type::post_draw_cards>(key, [=](player *p) {
                        if (p == origin) {
                            origin->add_to_hand_phase_one(origin->m_game->phase_one_drawn_card());
                            origin->m_game->remove_listeners(key);
                        }
                    });
                }
            }
        });
    }

    void effect_kit_carlson::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin) {
            if (target == origin && target->m_num_cards_to_draw < 3) {
                target->m_game->pop_request();
                for (int i=0; i<3; ++i) {
                    target->m_game->draw_phase_one_card_to(pocket_type::selection, target);
                }
                target->m_game->queue_request<request_kit_carlson>(target_card, target);
            }
        });
    }

    void request_kit_carlson::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->add_to_hand_phase_one(target_card);
        if (target->m_num_drawn_cards >= target->m_num_cards_to_draw) {
            target->m_game->pop_request();
            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::main_deck, nullptr, show_card_flags::hidden);
            }
        }
        target->m_game->update_request();
    }

    game_string request_kit_carlson::status_text(player *owner) const {
        if (owner == target) {
            return {"STATUS_KIT_CARLSON", origin_card};
        } else {
            return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
        }
    }

    void effect_el_gringo::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::after_hit>({target_card, 2}, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (origin && p == target && p->m_game->m_playing != p && !origin->m_hand.empty()) {
                target->m_game->flash_card(target_card);
                card *stolen_card = origin->random_hand_card();
                target->m_game->add_log(update_target::includes(origin, target), "LOG_STOLEN_CARD", target, origin, stolen_card);
                target->m_game->add_log(update_target::excludes(origin, target), "LOG_STOLEN_CARD_FROM_HAND", target, origin);
                target->steal_card(stolen_card);
                target->m_game->call_event<event_type::on_effect_end>(p, target_card);
            }
        });
    }

    void effect_suzy_lafayette::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_effect_end>(origin_card, [origin, origin_card](player *, card *) {
            origin->m_game->queue_action([origin, origin_card]{
                if (origin->alive() && origin->m_hand.empty()) {
                    origin->m_game->flash_card(origin_card);
                    origin->draw_card(1, origin_card);
                }
            });
        });
    }

    void effect_vulture_sam::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::verify_card_taker>(target_card, [=](player *e_target, equip_type type, bool &value){
            if (type == equip_type::vulture_sam && e_target == p) {
                value = true;
            }
        });
        p->m_game->add_listener<event_type::on_player_death>(target_card, [=](player *origin, player *target) {
            std::vector<player *> range_targets;
            int count = origin->m_game->num_alive();
            player_iterator it{target};
            do {
                ++it;
                bool valid = false;
                origin->m_game->call_event<event_type::verify_card_taker>(it, equip_type::vulture_sam, valid);
                if (valid) {
                    range_targets.push_back(it);
                }
            } while (--count != 0);
            if (range_targets.size() == 1) {
                std::vector<card *> target_cards;
                for (card *c : target->m_table) {
                    if (c->color != card_color_type::black) {
                        target_cards.push_back(c);
                    }
                }
                for (card *c : target->m_hand) {
                    target_cards.push_back(c);
                }

                for (card *c : target_cards) {
                    if (c->pocket == pocket_type::player_hand) {
                        target->m_game->add_log(update_target::includes(target, p), "LOG_STOLEN_CARD", p, target, c);
                        target->m_game->add_log(update_target::excludes(target, p), "LOG_STOLEN_CARD_FROM_HAND", p, target);
                    } else {
                        target->m_game->add_log("LOG_STOLEN_CARD", p, target, c);
                    }
                    p->steal_card(c);
                }
            } else if (!range_targets.empty() && range_targets.front() == p) {
                p->m_game->queue_request_front<request_multi_vulture_sam>(target_card, target, p, effect_flags::auto_pick);
            }
        });
    }

    bool effect_sid_ketchum::can_respond(card *origin_card, player *origin) {
        return origin->m_hand.size() >= 2 && effect_deathsave::can_respond(origin_card, origin);
    }

}