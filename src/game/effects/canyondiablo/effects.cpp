#include "effects.h"

#include "../valleyofshadows/requests.h"
#include "../base/requests.h"

#include "../../game.h"

namespace banggame {
    using namespace enums::flag_operators;

    void effect_graverobber::on_play(card *origin_card, player *origin) {
        for (int i=0; i<origin->m_game->num_alive(); ++i) {
            if (origin->m_game->m_discards.empty()) {
                origin->m_game->draw_card_to(pocket_type::selection);
            } else {
                origin->m_game->move_card(origin->m_game->m_discards.back(), pocket_type::selection);
            }
        }
        origin->m_game->queue_request<request_generalstore>(origin_card, origin, origin);
    }

    game_string effect_mirage::verify(card *origin_card, player *origin) {
        if (!origin->m_game->pending_requests()
            || origin->m_game->top_request().origin() != origin->m_game->m_playing) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }
        return {};
    }

    void effect_mirage::on_play(card *origin_card, player *origin) {
        origin->m_game->add_log("LOG_SKIP_TURN", origin->m_game->m_playing);
        origin->m_game->m_playing->skip_turn();
    }

    game_string effect_disarm::verify(card *origin_card, player *origin) {
        if (!origin->m_game->pending_requests() || !origin->m_game->top_request().origin()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }
        return {};
    }

    void effect_disarm::on_play(card *origin_card, player *origin) {
        player *shooter = origin->m_game->top_request().origin();
        if (!shooter->m_hand.empty()) {
            card *hand_card = shooter->random_hand_card();
            origin->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, shooter, hand_card);
            shooter->discard_card(hand_card);
        }
    }

    game_string handler_card_sharper::verify(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        if (auto *c = origin->find_equipped_card(target_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        if (auto *c = target_card->owner->find_equipped_card(chosen_card)) {
            return {"ERROR_DUPLICATED_CARD", c};
        }
        return {};
    }

    struct request_card_sharper : request_targeting {
        request_card_sharper(card *origin_card, player *origin, player *target, card *chosen_card, card *target_card)
            : request_targeting(origin_card, origin, target, target_card, effect_flags::escapable)
            , chosen_card(chosen_card) {}

        card *chosen_card;

        void on_finished() override {
            handler_card_sharper{}.on_resolve(origin_card, origin, chosen_card, target_card);
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CARD_SHARPER", origin_card, target_card, chosen_card};
            } else {
                return {"STATUS_CARD_SHARPER_OTHER", target, origin_card, target_card, chosen_card};
            }
        }
    };

    void handler_card_sharper::on_play(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        if (target_card->owner->can_escape(origin, origin_card, effect_flags::escapable)) {
            origin->m_game->queue_request<request_card_sharper>(origin_card, origin, target_card->owner, chosen_card, target_card);
        } else {
            on_resolve(origin_card, origin, chosen_card, target_card);
        }
    }

    void handler_card_sharper::on_resolve(card *origin_card, player *origin, card *chosen_card, card *target_card) {
        player *target = target_card->owner;
        origin->m_game->add_log("LOG_SWAP_CARDS", origin, target, chosen_card, target_card);

        target->disable_equip(target_card);
        target_card->on_equip(origin);
        origin->equip_card(target_card);
        if (chosen_card->owner == origin) {
            origin->disable_equip(chosen_card);
        }
        chosen_card->on_equip(target);
        target->equip_card(chosen_card);
    }

    bool effect_sacrifice::can_respond(card *origin_card, player *origin) {
        if (auto *req = origin->m_game->top_request_if<timer_damaging>()) {
            return req->target != origin;
        }
        return false;
    }

    void effect_sacrifice::on_play(card *origin_card, player *origin) {
        auto &req = origin->m_game->top_request().get<timer_damaging>();
        player *saved = req.target;
        bool fatal = saved->m_hp <= req.damage;
        if (0 == --req.damage) {
            origin->m_game->pop_request();
        }
        origin->damage(origin_card, origin, 1);
        origin->m_game->queue_action_front([=]{
            if (origin->alive()) {
                origin->draw_card(2 + fatal, origin_card);
            }
        });
        origin->m_game->update_request();
    }

    struct request_lastwill : request_base, resolvable_request {
        request_lastwill(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}

        void on_resolve() override {
            target->m_game->pop_request();
            target->m_game->update_request();
        }
        
        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_LASTWILL", origin_card};
            } else {
                return {"STATUS_LASTWILL_OTHER", origin_card, target};
            }
        }
    };

    void effect_lastwill::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_player_death_resolve>({origin_card, -1}, [=](player *target, bool tried_save) {
            if (origin == target) {
                target->m_game->queue_action_front([=]{
                    if (target->m_hp <= 0) {
                        origin->m_game->queue_request<request_lastwill>(origin_card, origin);
                    }
                });
            }
        });
    }

    bool effect_lastwill::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<request_lastwill>(origin);
    }

    void handler_lastwill::on_play(card *origin_card, player *origin, const target_list &targets) {
        player *target = targets[0].get<target_type::player>();

        origin->m_game->pop_request();

        for (auto c : targets | std::views::drop(1)) {
            card *chosen_card = c.get<target_type::card>();
            if (chosen_card->pocket == pocket_type::player_hand) {
                origin->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, chosen_card);
                origin->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
            } else {
                origin->m_game->add_log("LOG_GIFTED_CARD", origin, target, chosen_card);
            }
            target->add_to_hand(chosen_card);
        }

        origin->m_game->update_request();
    }
}