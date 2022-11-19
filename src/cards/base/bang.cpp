#include "bang.h"

#include "game/game.h"
#include "damage.h"

namespace banggame {
    
    void effect_bang::on_play(card *origin_card, player *origin, player *target, effect_flags flags) {
        if (!bool(flags & effect_flags::multi_target)) {
            target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        }
        target->m_game->queue_request<request_bang>(origin_card, origin, target, flags);
    }

    static void queue_request_bang(card *origin_card, player *origin, player *target, effect_flags flags = {}) {
        auto req = std::make_shared<request_bang>(origin_card, origin, target, flags | effect_flags::is_bang | effect_flags::single_target);
        req->origin->m_game->call_event<event_type::apply_bang_modifier>(req->origin, req.get());
        req->origin->m_game->queue_action([req = std::move(req)]() mutable {
            if (!req->target->immune_to(req->origin_card, req->origin, req->flags)) {
                req->origin->m_game->queue_request(std::move(req));
            }
        });
    }

    void handler_bangcard::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        queue_request_bang(origin_card, origin, target);
    }

    void handler_play_as_bang::on_play(card *origin_card, player *origin, card *chosen_card, player *target) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_BANG_ON", chosen_card, origin, target);
        origin->discard_card(chosen_card);
        queue_request_bang(chosen_card, origin, target, effect_flags::play_as_bang);
    }
    
    game_string effect_banglimit::verify(card *origin_card, player *origin) {
        if (origin->get_bangs_played() >= 1) {
            return "ERROR_ONE_BANG_PER_TURN";
        }
        return {};
    }

    void effect_banglimit::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 4};
        origin->m_game->add_listener<event_type::count_bangs_played>(key, [=](player *p, int &value) {
            if (origin == p) {
                ++value;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p, bool skipped) {
            if (origin == p) {
                origin->m_game->remove_listeners(key);
            }
        });
    }
    
    bool request_bang::can_miss(card *c) const {
        return !unavoidable && missable_request::can_miss(c);
    }

    void request_bang::on_miss() {
        auto target = this->target;
        if (--bang_strength == 0) {
            target->m_game->call_event<event_type::on_missed>(origin_card, origin, target, flags);
            target->m_game->pop_request();
        }
        target->m_game->update_request();
    }

    void request_bang::on_resolve() {
        target->m_game->pop_request();
        target->damage(origin_card, origin, bang_damage, flags);
        if (auto *req = target->m_game->top_request_if<timer_damaging>(target)) {
            static_cast<cleanup_request &>(*req) = std::move(*this);
        } else {
            target->m_game->update_request();
        }
    }
     
    void request_bang::set_unavoidable() {
        unavoidable = true;
        flags |= effect_flags::auto_respond;
    }

    bool request_bang::auto_resolve() {
        if (!num_cards_used()) {
            if (bool(flags & effect_flags::multi_target)) {
                target->m_game->play_sound(target, "gatling");
            } else {
                target->m_game->play_sound(target, "bang");
            }
        }
        return request_base::auto_resolve();
    }

    game_string request_bang::status_text(player *owner) const {
        if (bool(flags & effect_flags::play_as_bang)) {
            if (bool(flags & effect_flags::multi_target)) {
                if (target != owner) {
                    return {"STATUS_CARD_AS_GATLING_OTHER", target, origin_card};
                } else {
                    return {"STATUS_CARD_AS_GATLING", origin_card};
                }
            } else if (target != owner) {
                return {"STATUS_CARD_AS_BANG_OTHER", target, origin_card};
            } else if (bang_strength > 1) {
                if (bang_damage > 1) {
                    return {"STATUS_CARD_AS_BANG_DAMAGE_STRONG", origin_card, bang_strength, bang_damage};
                } else {
                    return {"STATUS_CARD_AS_BANG_STRONG", origin_card, bang_strength};
                }
            } else {
                if (bang_damage > 1) {
                    return {"STATUS_CARD_AS_BANG_DAMAGE", origin_card, bang_damage};
                } else {
                    return {"STATUS_CARD_AS_BANG", origin_card};
                }
            }
        } else {
            if (target != owner) {
                return {"STATUS_BANG_OTHER", target, origin_card};
            } else if (bang_strength > 1) {
                if (bang_damage > 1) {
                    return {"STATUS_BANG_DAMAGE_STRONG", origin_card, bang_strength, bang_damage};
                } else {
                    return {"STATUS_BANG_STRONG", origin_card, bang_strength};
                }
            } else {
                if (bang_damage > 1) {
                    return {"STATUS_BANG_DAMAGE", origin_card, bang_damage};
                } else {
                    return {"STATUS_BANG", origin_card};
                }
            }
        }
    }

}