#include "effects.h"
#include "equips.h"
#include "requests.h"

#include "../base/effects.h"
#include "../base/requests.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void handler_draw_atend::on_play(card *origin_card, player *origin, int amount) {
        if (amount > 0) {
            effect_draw(amount).on_play(origin_card, origin);
        }
    }

    opt_game_str handler_heal_multi::on_prompt(card *origin_card, player *origin, int amount) {
        return effect_heal(amount).on_prompt(origin_card, origin);
    }

    void handler_heal_multi::on_play(card *origin_card, player *origin, int amount) {
        effect_heal(amount).on_play(origin_card, origin);
    }

    opt_game_str effect_select_cube::verify(card *origin_card, player *origin, card *target) {
        if (target->num_cubes == 0) {
            return game_string("ERROR_NOT_ENOUGH_CUBES_ON", target);
        }
        return std::nullopt;
    }

    void effect_select_cube::on_play(card *origin_card, player *origin, card *target) {
        target->owner->pay_cubes(target, 1);
    }

    opt_game_str effect_pay_cube::verify(card *origin_card, player *origin) {
        if (origin_card->num_cubes < ncubes) {
            return game_string("ERROR_NOT_ENOUGH_CUBES_ON", origin_card);
        }
        return std::nullopt;
    }

    void effect_pay_cube::on_play(card *origin_card, player *origin) {
        origin->pay_cubes(origin_card, ncubes);
    }

    void effect_add_cube::on_play(card *origin_card, player *origin, card *target) {
        target->owner->add_cubes(target, ncubes);
    }

    void effect_reload::on_play(card *origin_card, player *origin) {
        origin->queue_request_add_cube(origin_card, 3);
    }

    opt_game_str effect_rust::on_prompt(card *origin_card, player *origin, player *target) {
        if (target->count_cubes() == 0) {
            return game_string{"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return std::nullopt;
        }
    }
    
    void effect_rust::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (target->count_cubes() == 0) return;
        origin->m_game->queue_action([=]{
            if (target->can_escape(origin, origin_card, flags)) {
                origin->m_game->queue_request<request_rust>(origin_card, origin, target, flags);
            } else {
                effect_rust{}.on_resolve(origin_card, origin, target);
            }
        });
    }

    void effect_rust::on_resolve(card *origin_card, player *origin, player *target) {
        auto view = target->m_table | std::views::filter([](card *c){ return c->color == card_color_type::orange; });
        std::vector<card *> orange_cards{view.begin(), view.end()};
        
        orange_cards.push_back(target->m_characters.front());
        for (card *c : orange_cards) {
            target->move_cubes(c, origin->m_characters.front(), 1);
        }

        target->m_game->call_event<event_type::on_effect_end>(origin, origin_card);
    }

    void effect_doublebarrel::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                if (origin->get_card_sign(req->origin_card).suit == card_suit::diamonds) {
                    req->set_unavoidable();
                }
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }

    void effect_thunderer::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                req->origin->m_game->add_log("LOG_STOLEN_SELF_CARD", req->origin, req->origin_card);
                req->origin->m_game->move_card(req->origin_card, pocket_type::player_hand, req->origin, show_card_flags::short_pause);
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }

    void effect_buntlinespecial::on_play(card *origin_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *origin, request_bang *req) {
            if (p == origin) {
                p->m_game->add_listener<event_type::on_missed>(origin_card, [=](card *bang_card, player *origin, player *target, bool is_bang) {
                    if (target && origin == p && is_bang && !target->m_hand.empty()) {
                        target->m_game->queue_request<request_discard>(origin_card, origin, target);
                    }
                });
                req->on_cleanup([=]{
                    p->m_game->remove_listeners(origin_card);
                });
            }
        });
    }

    void effect_bigfifty::on_play(card *origin_card, player *p) {
        p->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *origin, request_bang *req) {
            if (origin == p) {
                origin->m_game->add_disabler(origin_card, [target = req->target](card *c) {
                    return (c->pocket == pocket_type::player_table
                        || c->pocket == pocket_type::player_character)
                        && c->owner == target;
                });
                req->on_cleanup([=]{
                    p->m_game->remove_disablers(origin_card);
                });
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }

    void handler_flintlock::on_play(card *origin_card, player *origin, player *target, opt_tagged_value<target_type::none> paid_cubes) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target, effect_flags::escapable | effect_flags::single_target);
        if (paid_cubes) {
            origin->m_game->add_listener<event_type::on_missed>(origin_card, [=](card *origin_card, player *p, player *target, bool is_bang) {
                if (origin == p) {
                    origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
                    origin->add_to_hand(origin_card);
                }
            });
            req->on_cleanup([=]{
                origin->m_game->remove_listeners(origin_card);
            });
        }
        origin->m_game->queue_request(std::move(req));
    }

    opt_game_str effect_bandolier::verify(card *origin_card, player *origin) {
        if (origin->m_bangs_played == 0) {
            return game_string("ERROR_CANT_PLAY_CARD", origin_card);
        }
        return std::nullopt;
    }

    void handler_duck::on_play(card *origin_card, player *origin, opt_tagged_value<target_type::none> paid_cubes) {
        if (paid_cubes) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
            origin->add_to_hand(origin_card);
        }
        effect_missed().on_play(origin_card, origin);
    }

    void handler_squaw::on_play(card *origin_card, player *origin, card *target_card, opt_tagged_value<target_type::none> paid_cubes) {
        const auto flags = effect_flags::escapable | effect_flags::single_target;
        if (target_card->owner && !target_card->owner->immune_to(origin_card, origin, flags)) {
            if (paid_cubes) {
                effect_steal{}.on_play(origin_card, origin, target_card, flags);
            } else {
                effect_destroy{}.on_play(origin_card, origin, target_card, flags);
            }
        }
    }

    void effect_tumbleweed::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_check_select>(target_card, [=](player *origin, card *origin_card, card *drawn_card) {
            target->m_game->queue_request_front<timer_tumbleweed>(target_card, origin, target, drawn_card, origin_card);
        });
    }

    bool effect_tumbleweed::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<timer_tumbleweed>(origin);
    }

    void effect_tumbleweed::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
        origin->m_game->m_current_check.restart();
        origin->m_game->update_request();
    }

    void timer_tumbleweed::on_finished() {
        origin->m_game->m_current_check.resolve(drawn_card);
    }

    bool effect_move_bomb::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<request_move_bomb>(origin);
    }

    opt_game_str handler_move_bomb::on_prompt(card *origin_card, player *origin, player *target) {
        if (origin == target) {
            return game_string{"PROMPT_MOVE_BOMB_TO_SELF", origin_card};
        } else {
            return std::nullopt;
        }
    }

    opt_game_str handler_move_bomb::verify(card *origin_card, player *origin, player *target) {
        if (target != origin) {
            if (auto c = target->find_equipped_card(origin_card)) {
                return game_string("ERROR_DUPLICATED_CARD", c);
            }
        }
        return std::nullopt;
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
}