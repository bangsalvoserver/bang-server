#include "effects.h"
#include "requests.h"

#include "../base/effects.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    opt_error effect_aim::verify(card *origin_card, player *origin) {
        if (auto error = effect_banglimit{}.verify(origin_card, origin))
            return error;

        auto is_bangcard = [origin](card *c) {
            return origin->is_bangcard(c) || c->has_tag(tag_type::play_as_bang);
        };

        constexpr effect_holder bang_holder{
            .player_filter{target_player_filter::reachable | target_player_filter::notself}
        };
        
        if (std::ranges::none_of(origin->m_hand, is_bangcard)
         || std::ranges::none_of(origin->m_table, is_bangcard)
         || std::ranges::none_of(origin->m_characters, is_bangcard)

         || origin->make_player_target_set(origin_card, bang_holder).empty())
            return game_error("ERROR_INVALID_ACTION");
        
        return std::nullopt;
    }

    void effect_aim::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                ++req->bang_damage;
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }
    
    opt_error effect_backfire::verify(card *origin_card, player *origin) {
        if (!origin->m_game->pending_requests() || !origin->m_game->top_request().origin()) {
            return game_error("ERROR_CANT_PLAY_CARD", origin_card);
        }
        return std::nullopt;
    }

    void effect_backfire::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_bang>(origin_card, origin, origin->m_game->top_request().origin(),
            effect_flags::escapable | effect_flags::single_target);
    }

    void effect_bandidos::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        target->m_game->queue_request<request_bandidos>(origin_card, origin, target, flags);
    }

    void effect_tornado::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (target->m_hand.empty()) {
            target->m_game->queue_action([=]{
                target->draw_card(2, origin_card);
            });
        } else {
            // ignore flags ... why would you ever play escape vs tornado?
            target->m_game->queue_request<request_tornado>(origin_card, origin, target);
        }
    }

    void effect_poker::on_play(card *origin_card, player *origin) {
        auto targets = range_other_players(origin) | std::views::filter([](const player &p) {
            return !p.m_hand.empty();
        });

        effect_flags flags = effect_flags::escapable;
        if (std::ranges::distance(targets) == 1) flags |= effect_flags::single_target;
        
        for (player &p : targets) {
            origin->m_game->queue_request<request_poker>(origin_card, origin, &p, flags);
        }
        origin->m_game->queue_action([=]{
            for (auto it = origin->m_game->m_selection.begin(); it != origin->m_game->m_selection.end(); ++it) {
                origin->m_game->add_log("LOG_POKER_REVEAL", origin_card, *it);
                auto flags = show_card_flags::shown;
                if (std::next(it) == origin->m_game->m_selection.end()) {
                    flags |= show_card_flags::short_pause;
                }
                origin->m_game->send_card_update(*it, nullptr, flags);
            }
            if (std::ranges::any_of(origin->m_game->m_selection, [origin](card *card_ptr) {
                return origin->get_card_sign(card_ptr).rank == card_rank::rank_A;
            })) {
                origin->m_game->add_log("LOG_POKER_ACE");
                while (!origin->m_game->m_selection.empty()) {
                    origin->m_game->move_card(origin->m_game->m_selection.front(), pocket_type::discard_pile);
                }
            } else if (origin->m_game->m_selection.size() <= 2) {
                while (!origin->m_game->m_selection.empty()) {
                    card *drawn_card = origin->m_game->m_selection.front();
                    origin->m_game->add_log("LOG_DRAWN_CARD", origin, drawn_card);
                    origin->add_to_hand(drawn_card);
                }
            } else {
                origin->m_game->queue_request<request_poker_draw>(origin_card, origin);
            }
        });
    }

    bool effect_saved::can_respond(card *origin_card, player *origin) {
        if (auto *req = origin->m_game->top_request_if<timer_damaging>()) {
            return req->target != origin;
        }
        return false;
    }

    void effect_saved::on_play(card *origin_card, player *origin) {
        auto &req = origin->m_game->top_request().get<timer_damaging>();
        player *saved = req.target;
        origin->m_game->queue_action_front([=]{
            if (saved->alive()) {
                origin->m_game->queue_request<request_saved>(origin_card, origin, saved);
            }
        });
        if (0 == --req.damage) {
            origin->m_game->pop_request();
        }
        origin->m_game->update_request();
    }

    bool effect_escape::can_respond(card *origin_card, player *origin) {
        return origin->m_game->pending_requests() && origin->m_game->top_request().target() == origin
            && bool(origin->m_game->top_request().flags() & effect_flags::escapable);
    }

    void effect_escape::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
        origin->m_game->update_request();
    }

    opt_error handler_fanning::verify(card *origin_card, player *origin, player *player1, player *player2) {
        if (player1 == player2 || origin->m_game->calc_distance(player1, player2) > 1) {
            return game_error("ERROR_TARGET_NOT_IN_RANGE");
        }
        return std::nullopt;
    }

    void handler_fanning::on_play(card *origin_card, player *origin, player *player1, player *player2) {
        effect_bang{}.on_play(origin_card, origin, player1, effect_flags::escapable);
        effect_bang{}.on_play(origin_card, origin, player2, effect_flags::escapable);
    }

    void handler_play_as_gatling::on_play(card *origin_card, player *origin, card *chosen_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_GATLING", chosen_card, origin);
        origin->discard_card(chosen_card);

        auto targets = range_other_players(origin);
        auto flags = std::ranges::distance(targets) == 1 ? effect_flags::single_target : effect_flags{};
        for (player &p : targets) {
            if (!p.immune_to(chosen_card)) {
                origin->m_game->queue_request<request_card_as_gatling>(chosen_card, origin, &p, flags);
            }
        }
    }

}