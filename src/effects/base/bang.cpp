#include "bang.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/filters.h"
#include "game/play_verify.h"
#include "game/prompts.h"

#include "damage.h"

namespace banggame {

    prompt_string effect_bang::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_kill_sheriff(origin, target));
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }
    
    void effect_bang::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) {
        if (!flags.check(effect_flag::target_players)) {
            target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        }
        target->m_game->queue_request<request_bang>(origin_card, origin, target, flags);
    }

    game_string effect_bangcard::get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        if (origin_card->has_tag(tag_type::bangcard) && origin->m_game->check_flags(game_flag::showdown)) {
            return "ERROR_CARD_INACTIVE";
        } else if (!ctx.disable_bang_checks) {
            game_string out_error;
            origin->m_game->call_event(event_type::check_bang_target{ origin_card, origin, target, effect_flag::is_bang, out_error });
            return out_error;
        } else {
            return {};
        }
    }

    game_string effect_bangcard::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }

    void effect_bangcard::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        if (flags.check(effect_flag::play_as_bang)) {
            origin->m_game->add_log("LOG_PLAYED_CARD_AS_BANG_ON", origin_card, origin, target);
        } else {
            origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        }
        
        flags.add(effect_flag::is_bang);
        
        auto req = std::make_shared<request_bang>(origin_card, origin, target, flags);
        req->origin->m_game->call_event(event_type::apply_bang_modifier{ req->origin, req });
        req->origin->m_game->queue_request(std::move(req));
    }

    prompt_string effect_play_as_bang::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        return effect_bang{}.on_prompt(ctx.playing_card, origin, target);
    }

    void effect_play_as_bang::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) {
        flags.add(effect_flag::play_as_bang);
        effect_bang{}.on_play(ctx.playing_card, origin, target, flags);
    }

    game_string effect_play_as_bangcard::get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        return effect_bangcard{}.get_error(ctx.playing_card, origin, target, ctx);
    }

    prompt_string effect_play_as_bangcard::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        return effect_bangcard{}.on_prompt(ctx.playing_card, origin, target);
    }

    void effect_play_as_bangcard::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) {
        flags.add(effect_flag::play_as_bang);
        effect_bangcard{}.on_play(ctx.playing_card, origin, target, flags);
    }
    
    game_string effect_banglimit::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (!ctx.disable_banglimit && origin->get_bangs_played() >= 1) {
            return "ERROR_ONE_BANG_PER_TURN";
        }
        return {};
    }

    void effect_banglimit::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (ctx.disable_banglimit) {
            return;
        }
        event_card_key key{origin_card, 4};
        origin->m_game->add_listener<event_type::count_bangs_played>(key, [=](const_player_ptr p, int &value) {
            if (origin == p) {
                ++value;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr p, bool skipped) {
            if (origin == p) {
                origin->m_game->remove_listeners(key);
            }
        });
    }

    bool modifier_bangmod::valid_with_modifier(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return target_card->has_tag(tag_type::bangmod);
    }

    bool modifier_bangmod::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (target_card->pocket == pocket_type::player_hand) {
            return target_card->has_tag(tag_type::bangcard);
        } else {
            return target_card->has_tag(tag_type::play_as_bang);
        }
    }
    
    bool request_bang::can_miss(card_ptr c) const {
        return !unavoidable && missable_request::can_miss(c);
    }

    void request_bang::on_miss(card_ptr missed_card, effect_flags missed_flags) {
        if (--bang_strength == 0) {
            missed_flags.merge(flags);
            target->m_game->call_event(event_type::on_missed{ missed_card, missed_flags, shared_from_this() });
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
            if (update_count == 0) {
                if (flags.check(effect_flag::target_players)) {
                    target->play_sound(sound_id::gatling);
                } else {
                    target->play_sound(sound_id::bang);
                }
            }
            if (!unavoidable && bang_strength == 0) {
                target->m_game->pop_request();
            } else if (unavoidable || target->empty_hand()) {
                auto_resolve();
            }
        }
    }

    game_string request_bang::status_text(player_ptr owner) const {
        if (flags.check(effect_flag::play_as_bang)) {
            if (flags.check(effect_flag::target_players)) {
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

    bool effect_bangresponse::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<respondable_with_bang>(target_is{origin}) != nullptr;
    }

    void effect_bangresponse::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<respondable_with_bang>();
        req->respond_with_bang();
    }

}