#include "scenarios.h"

#include "../base/requests.h"
#include "../base/effects.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_ambush::on_enable(card *target_card, player *target) {
        target->m_game->set_game_flags(game_flags::disable_player_distances);
    }

    void effect_sniper::on_play(card *origin_card, player *origin, player *target) {
        target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target);
        req->bang_strength = 2;
        target->m_game->queue_request(std::move(req));
    }

    game_string effect_startofturn::verify(card *origin_card, player *origin) const {
        if (origin->m_num_drawn_cards != 0) {
            return "ERROR_NOT_START_OF_TURN";
        }
        return {};
    }

    struct request_ranch : request_base {
        request_ranch(card *target_card, player *target)
            : request_base(target_card, nullptr, target, effect_flags::auto_respond) {}

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_RANCH", origin_card};
            } else {
                return {"STATUS_RANCH_OTHER", origin_card, target};
            }
        }
    };

    void effect_ranch::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::post_draw_cards>(target_card, [=](player *origin) {
            origin->m_game->queue_request<request_ranch>(target_card, origin);
        });
    }

    bool effect_ranch::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<request_ranch>(origin);
    }

    void effect_ranch::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
        origin->m_game->update_request();
    }

    void effect_deadman::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::verify_revivers>(target_card, [=](player *target) {
            if (!target->alive() && target == origin->m_game->m_first_dead) {
                target->m_game->flash_card(target_card);
                origin->m_game->add_log("LOG_REVIVE", target, target_card);

                target->remove_player_flags(player_flags::dead);
                target->set_hp(2);
                target->draw_card(2);

                for (auto *c : target->m_characters) {
                    c->on_enable(target);
                }
            }
        });
    }

    void effect_judge::on_enable(card *target_card, player *target) {
        target->m_game->set_game_flags(game_flags::disable_equipping);
    }

    void effect_lasso::on_enable(card *target_card, player *target) {
        target->m_game->add_disabler(target_card, [](card *c) {
            return c->pocket == pocket_type::player_table;
        });
    }

    void effect_lasso::on_disable(card *target_card, player *target) {
        target->m_game->remove_disablers(target_card);
    }

    void effect_abandonedmine::on_enable(card *target_card, player *target) {
        target->m_game->set_game_flags(game_flags::phase_one_draw_discard);
    }

    void effect_peyote::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::phase_one_override>(target_card, [=](player *p) {
            std::vector<card *> target_cards;
            for (card *c : p->m_game->m_hidden_deck) {
                if (c->has_tag(tag_type::peyote)) {
                    target_cards.push_back(c);
                }
            }
            for (card *c : target_cards) {
                p->m_game->move_card(c, pocket_type::selection, nullptr, show_card_flags::instant);
            }
            
            p->m_game->queue_request<request_peyote>(target_card, p);
        });

        target->m_game->set_game_flags(game_flags::phase_one_override);
    }

    void request_peyote::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->m_game->flash_card(target_card);
        
        auto *drawn_card = target->m_game->m_deck.back();
        target->m_game->send_card_update(drawn_card, nullptr, show_card_flags::short_pause);

        short choice = *target_card->get_tag_value(tag_type::peyote);

        if (choice == 1) {
            target->m_game->add_log("LOG_DECLARED_RED", target, origin_card);
        } else {
            target->m_game->add_log("LOG_DECLARED_BLACK", target, origin_card);
        }

        if (choice == 1
            ? (drawn_card->sign.suit == card_suit::hearts || drawn_card->sign.suit == card_suit::diamonds)
            : (drawn_card->sign.suit == card_suit::clubs || drawn_card->sign.suit == card_suit::spades))
        {
            target->draw_card();
        } else {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target->m_game->m_deck.back());
            target->m_game->draw_card_to(pocket_type::discard_pile);

            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::hidden_deck, nullptr, show_card_flags::instant);
            }
            target->m_game->call_event<event_type::post_draw_cards>(target);
            target->m_game->update_request();
        }
    }

    game_string request_peyote::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_PEYOTE", origin_card};
        } else {
            return {"STATUS_PEYOTE_OTHER", target, origin_card};
        }
    }

    void effect_ricochet::on_play(card *origin_card, player *origin, card *target_card) {
        origin->m_game->queue_request<request_ricochet>(origin_card, origin, target_card->owner, target_card);
    }

    void request_ricochet::on_resolve() {
        origin->m_game->pop_request();
        effect_destroy::resolver{origin_card, origin, target_card}.resolve();
        origin->m_game->update_request();
    }

    void request_ricochet::on_miss() {
        auto target = this->target;
        target->m_game->pop_request();
        target->m_game->update_request();
    }

    game_string request_ricochet::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_RICOCHET", origin_card, target_card};
        } else {
            return {"STATUS_RICOCHET_OTHER", target, origin_card, target_card};
        }
    }

    void effect_russianroulette::on_enable(card *target_card, player *target) {
        auto queue_russianroulette_request = [=](player *target) {
            auto req = std::make_shared<request_bang>(target_card, nullptr, target);
            req->bang_damage = 2;
            target->m_game->queue_request(std::move(req));
        };
        queue_russianroulette_request(target);
        target->m_game->add_listener<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, bool is_bang) {
            if (target_card == origin_card) {
                queue_russianroulette_request(std::next(player_iterator(target)));
            }
        });
        target->m_game->add_listener<event_type::before_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (target_card == origin_card) {
                target->m_game->remove_listeners(target_card);
            }
        });
    }

    void effect_fistfulofcards::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player *p) {
            p->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", p, target_card, static_cast<int>(p->m_hand.size()));
            for (int i=0; i<p->m_hand.size(); ++i) {
                p->m_game->queue_action([=]{
                    if (p->alive()) {
                        p->m_game->queue_request<request_bang>(target_card, nullptr, p);
                    }
                });
            }
        });
    }

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
                origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p) {
                    if (p == origin) {
                        origin->m_game->remove_listeners(key);
                    }
                });
            }
        });
    }

    void effect_vendetta::on_enable(card *target_card, player *p) {
        p->m_game->add_listener<event_type::post_turn_end>({target_card, 2}, [target_card](player *target) {
            target->m_game->queue_action([target, target_card] {
                target->m_game->draw_check_then(target, target_card, [target, target_card](card *drawn_card) {
                    if (target->get_card_sign(drawn_card).suit == card_suit::hearts) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                        ++target->m_extra_turns;
                    }
                });
            });
        });
    }
}