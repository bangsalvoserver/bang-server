#include "bandidos.h"

#include "game/game.h"
#include "game/prompts.h"
#include "game/possible_to_play.h"

#include "cards/game_enums.h"

#include "effects/base/requests.h"

namespace banggame {

    struct request_bandidos : request_resolvable, interface_picking {
        request_bandidos(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags) {}

        int ncards = 0;

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (!live) {
                    target->play_sound("bandidos");
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
                return "BOT_BAD_PLAY";
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
            return { prompt_type::priority, "PROMPT_SUICIDE", origin_card };
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
                if (!live) {
                    target->play_sound("bandidos");
                }
                auto not_disabled = target->m_hand
                    | rv::remove_if([&](const_card_ptr c) {
                        return target->m_game->is_usage_disabled(c);
                    })
                    | rn::to_vector;
                if (not_disabled.size() <= 1) {
                    handler_bandidos2_response{}.on_play(origin_card, target, not_disabled);
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
        return origin->m_game->top_request<request_bandidos2>(origin) != nullptr;
    }

    game_string handler_bandidos2_response::get_error(card_ptr origin_card, player_ptr origin, const card_list &target_cards) {
        for (card_ptr target_card : target_cards) {
            if (card_ptr disabler = origin->m_game->get_usage_disabler(target_card)) {
                return {"ERROR_CARD_DISABLED_BY", target_card, disabler};
            }
        }
        if (target_cards.size() == 1 && !target_cards.front()->is_bang_card(origin)) {
            return "ERROR_TARGET_NOT_BANG";
        }
        return {};
    }

    void handler_bandidos2_response::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards) {
        origin->m_game->pop_request();

        for (card_ptr target_card : target_cards) {
            origin->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, origin, target_card);
            origin->discard_used_card(target_card);
        }
    }
}