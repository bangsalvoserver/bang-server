#include "bang.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"
#include "game/filters.h"

#include "game/game.h"
#include "game/play_verify.h"
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
        req->origin->m_game->call_event(event_type::apply_bang_modifier{ req->origin, req });
        req->origin->m_game->queue_request(std::move(req));
    }

    game_string effect_bangcard::get_error(card *origin_card, player *origin, player *target, const effect_context &ctx) {
        if (origin_card->has_tag(tag_type::bangcard) && origin->m_game->check_flags(game_flags::treat_any_as_bang)) {
            return "ERROR_CARD_INACTIVE";
        } else if (!ctx.disable_bang_checks) {
            game_string out_error;
            origin->m_game->call_event(event_type::check_bang_target{ origin_card, origin, target, effect_flags::is_bang, out_error });
            return out_error;
        } else {
            return {};
        }
    }

    void effect_bangcard::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        queue_request_bang(origin_card, origin, target);
    }

    bool handler_play_as_bang::on_check_target(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target) {
        return effect_bangcard{}.on_check_target(chosen_card, origin, target);
    }

    game_string handler_play_as_bang::get_error(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target) {
        MAYBE_RETURN(get_play_card_error(origin, chosen_card, ctx));
        return effect_bangcard{}.get_error(chosen_card, origin, target, ctx);
    }

    game_string handler_play_as_bang::on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target) {
        return effect_bangcard{}.on_prompt(chosen_card, origin, target);
    }

    void handler_play_as_bang::on_play(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card, player *target) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_BANG_ON", chosen_card, origin, target);
        origin->discard_used_card(chosen_card);
        queue_request_bang(chosen_card, origin, target, effect_flags::play_as_bang);
    }
    
    game_string effect_banglimit::get_error(card *origin_card, player *origin, const effect_context &ctx) {
        if (!ctx.disable_banglimit && origin->get_bangs_played() >= 1) {
            return "ERROR_ONE_BANG_PER_TURN";
        }
        return {};
    }

    void effect_banglimit::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        if (ctx.disable_banglimit) {
            return;
        }
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

    equip_treat_as_bang::equip_treat_as_bang(int value)
        : flag{value == 1 ? player_flags::treat_missed_as_bang : player_flags::treat_any_as_bang} {}

    void equip_treat_as_bang::on_enable(card *origin_card, player *p) {
        p->add_player_flags(flag);
    }

    void equip_treat_as_bang::on_disable(card *origin_card, player *p) {
        p->remove_player_flags(flag);
    }

    bool modifier_bangmod::valid_with_modifier(card *origin_card, player *origin, card *target_card) {
        return target_card->has_tag(tag_type::bangmod);
    }

    bool modifier_bangmod::valid_with_card(card *origin_card, player *origin, card *target_card) {
        if (target_card->pocket == pocket_type::player_hand) {
            return target_card->has_tag(tag_type::bangcard);
        } else {
            return target_card->has_tag(tag_type::play_as_bang);
        }
    }
    
    bool request_bang::can_miss(card *c) const {
        return !unavoidable && missable_request::can_miss(c);
    }

    void request_bang::on_miss(card *c, effect_flags missed_flags) {
        if (--bang_strength == 0) {
            target->m_game->call_event(event_type::on_missed{ origin_card, origin, target, c, flags | missed_flags });
            target->m_game->pop_request();
        }
    }

    void request_bang::on_resolve() {
        target->m_game->pop_request();
        target->damage(origin_card, origin, bang_damage, flags);
    }

    void request_bang::on_update() {
        if (!target->alive() || target->immune_to(origin_card, origin, flags)) {
            target->m_game->pop_request();
        } else {
            if (!live) {
                if (bool(flags & effect_flags::multi_target)) {
                    target->m_game->play_sound(target, "gatling");
                } else {
                    target->m_game->play_sound(target, "bang");
                }
            }
            if (target->empty_hand() || unavoidable) {
                auto_resolve();
            }
        }
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
            } else {
                return {"STATUS_CARD_AS_BANG", origin_card, bang_strength, bang_damage};
            }
        } else {
            if (target != owner) {
                return {"STATUS_BANG_OTHER", target, origin_card};
            } else {
                return {"STATUS_BANG", origin_card, bang_strength, bang_damage};
            }
        }
    }

    bool effect_bangresponse::can_play(card *origin_card, player *origin) {
        return origin->m_game->top_request<respondable_with_bang>(origin) != nullptr;
    }

    void effect_bangresponse::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<respondable_with_bang>();
        req->respond_with_bang();
    }

}