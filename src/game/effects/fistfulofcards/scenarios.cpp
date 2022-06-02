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

    opt_error effect_startofturn::verify(card *origin_card, player *origin) const {
        if (!origin->check_player_flags(player_flags::start_of_turn)) {
            return game_error("ERROR_NOT_START_OF_TURN");
        }
        return std::nullopt;
    }

    void effect_deadman::on_enable(card *target_card, player *origin) {
        origin->m_game->add_event<event_type::verify_revivers>(target_card, [=](player *target) {
            if (!target->alive() && target == origin->m_game->m_first_dead) {
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
        target->m_game->add_event<event_type::on_request_draw>(target_card, [=](player *p) {
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
            target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target->m_game->m_deck.back());
            target->m_game->draw_card_to(pocket_type::discard_pile);

            while (!target->m_game->m_selection.empty()) {
                target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::hidden_deck, nullptr, show_card_flags::instant);
            }
            target->m_game->pop_request_update();
            target->m_game->call_event<event_type::post_draw_cards>(target);
        }
    }

    game_formatted_string request_peyote::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_PEYOTE", origin_card};
        } else {
            return {"STATUS_PEYOTE_OTHER", target, origin_card};
        }
    }

    void effect_ricochet::on_play(card *origin_card, player *origin, card *target_card) {
        origin->m_game->queue_request<request_ricochet>(origin_card, origin, target_card->owner, target_card);
    }

    void request_ricochet::on_finished() {
        effect_destroy::resolver{origin_card, origin, target_card}.resolve();
        origin->m_game->pop_request_update();
    }

    game_formatted_string request_ricochet::status_text(player *owner) const {
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
        target->m_game->add_event<event_type::on_missed>(target_card, [=](card *origin_card, player *origin, player *target, bool is_bang) {
            if (target_card == origin_card) {
                queue_russianroulette_request(std::next(player_iterator(target)));
            }
        });
        target->m_game->add_event<event_type::on_hit>(target_card, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (target_card == origin_card) {
                target->m_game->remove_events(target_card);
            }
        });
    }

    void effect_fistfulofcards::on_enable(card *target_card, player *target) {
        target->m_game->add_event<event_type::pre_turn_start>(target_card, [=](player *p) {
            p->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", p, target_card, p->m_hand.size());
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
        target->m_game->add_event<event_type::on_card_drawn>(target_card, [](player *origin, card *drawn_card, bool &reveal) {
            if (origin->m_num_drawn_cards == 2) {
                reveal = true;
                origin->m_game->queue_action([=]{
                    if (origin->is_possible_to_play(drawn_card)) {
                        origin->m_game->add_log("LOG_MANDATORY_CARD", origin, drawn_card);
                        origin->set_mandatory_card(drawn_card);
                    }
                });
            }
        });
    }

    void effect_vendetta::on_enable(card *target_card, player *p) {
        p->m_game->add_event<event_type::post_turn_end>({target_card, 2}, [target_card](player *target) {
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