#include "bandidos.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/possible_to_play.h"

#include "cards/game_enums.h"

#include "effects/base/requests.h"
#include "effects/base/escapable.h"
#include "effects/base/steal_destroy.h"

namespace banggame {

    struct request_bandidos : request_auto_resolvable, interface_picking, escapable_request {
        request_bandidos(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_auto_resolvable(origin_card, origin, target, flags) {}

        int ncards = 0;

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (update_count == 0) {
                    target->play_sound(sound_id::bandidos);
                }
                if (target->empty_hand()) {
                    auto_resolve();
                }
            }
        }

        void on_resolve() override {
            target->m_game->pop_request();
            if (ncards > 0) {
                target->reveal_hand();
            }
            target->damage(origin_card, origin, 1);
        }

        prompt_string resolve_prompt() const override {
            if (target->is_bot() && target->m_hp <= 1 && rn::any_of(target->m_hand, [&](card_ptr target_card) { return can_pick(target_card); })) {
                return "BOT_MUST_RESPOND_BANDIDOS";
            }
            return {};
        }
        
        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && !target->m_game->is_usage_disabled(target_card);
        }

        void on_pick(card_ptr target_card) override {
            ++ncards;
            
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_used_card(target_card);
            
            if (target->empty_hand()) {
                target->m_game->pop_request();
            } else if (rn::any_of(target->m_hand, [&](const_card_ptr c) { return can_pick(c); })) {
                target->m_game->pop_request();
                target->m_game->queue_request<request_discard>(origin_card, origin, target, effect_flags{}, 110);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_BANDIDOS", origin_card};
            } else {
                return {"STATUS_BANDIDOS_OTHER", target, origin_card};
            }
        }
    };

    prompt_string effect_bandidos::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) {
        MAYBE_RETURN(prompts::bot_check_kill_sheriff(origin, target));
        if (origin == target && target->m_hp <= 1 && target->m_hand.size() <= 1) {
            return {1, "PROMPT_SUICIDE", origin_card};
        }
        return {};
    }

    void effect_bandidos::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        target->m_game->queue_request<request_bandidos>(origin_card, origin, target, flags);
    }

    struct request_bandidos2 : request_base {
        using request_base::request_base;

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags) || target->empty_hand()) {
                target->m_game->pop_request();
            } else {
                if (update_count == 0) {
                    target->play_sound(sound_id::bandidos);
                }
                auto not_disabled = target->m_hand
                    | rv::remove_if([&](const_card_ptr c) {
                        return target->m_game->is_usage_disabled(c);
                    })
                    | rn::to_vector;
                if (not_disabled.size() <= 1) {
                    origin->m_game->pop_request();
                    for (card_ptr target_card : not_disabled) {
                        effect_discard{true}.on_play(origin_card, target, target_card);
                    }
                    target->reveal_hand();
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_BANDIDOS2", origin_card};
            } else {
                return {"STATUS_BANDIDOS2_OTHER", target, origin_card};
            }
        }
    };

    void effect_bandidos2::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        target->m_game->queue_request<request_bandidos2>(origin_card, origin, target, flags);
    }

    bool effect_bandidos2_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_bandidos2>(target_is{origin}) != nullptr;
    }

    void effect_bandidos2_response::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
}