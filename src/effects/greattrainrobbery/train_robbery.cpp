#include "train_robbery.h"

#include "game/game_table.h"
#include "game/bot_suggestion.h"
#include "game/prompts.h"

#include "effects/base/bang.h"
#include "effects/base/missed.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

namespace banggame {

    struct request_train_robbery : request_base, interface_target_set_cards, interface_escapable {
        using request_base::request_base;

        card_set selected_cards;

        bool can_escape(card_ptr c) const override {
            return selected_cards.empty() && interface_escapable::can_escape(c);
        }

        void on_update() override {
            if (!target->alive() || target->immune_to(origin_card, origin, flags)
                || rn::none_of(target->m_table, [&](card_ptr target_card) {
                    return in_target_set(target_card);
                })
            ) {
                pop_request();
            }
        }

        card_list get_highlights(player_ptr owner) const override {
            if (owner != target) {
                return target->m_table
                    | rv::filter([&](card_ptr target_card) { return in_target_set(target_card); }) 
                    | rn::to<std::vector>(); 
            }
            return {};
        }

        bool in_target_set(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target
                && !target_card->is_black() && !selected_cards.contains(target_card);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_TRAIN_ROBBERY", origin_card};
            } else {
                return {"STATUS_TRAIN_ROBBERY_OTHER", target, origin_card};
            }
        }
    };

    prompt_string effect_train_robbery::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) const {
        MAYBE_RETURN(prompts::prompt_target_immunity(origin_card, origin, target, flags));
        if (target->is_ghost()) {
            return {"PROMPT_TARGET_GHOST", origin_card, target};
        }
        if (rn::all_of(target->m_table, &card::is_black)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return {};
    }

    int effect_train_robbery::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        if (!bot_suggestion::is_target_enemy(origin, target)) {
            return -5;
        } else if (rn::any_of(target->m_table, [](card_ptr c) { return c->has_tag(tag_type::jail); })) {
            return -3;
        }
        return 0;
    }

    void effect_train_robbery::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) const {
        bot_suggestion::signal_hostile_action(origin, target, flags);

        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        origin->m_game->queue_request<request_train_robbery>(origin_card, origin, target, flags, 20);
    }

    bool effect_train_robbery_response::can_play(card_ptr origin_card, player_ptr origin) const {
        return origin->m_game->top_request<request_train_robbery>(target_is{origin}) != nullptr;
    }

    namespace contexts {
        struct train_robbery_target {
            card_ptr value;
        };
    }

    void effect_train_robbery_response::add_context(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_context &ctx) const {
        ctx.add(contexts::train_robbery_target{ target_card });
    }

    int effect_train_robbery_response::evaluate(card_ptr origin_card, player_ptr origin, card_ptr target_card) const {
        if (target_card->has_tag(tag_type::penalty)) {
            return 5;
        }
        return 0;
    }

    void effect_train_robbery_response::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) const {
        auto req = origin->m_game->top_request<request_train_robbery>();
        req->selected_cards.add(target_card);
    }

    int effect_train_robbery_discard::evaluate(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        if (ctx.get<contexts::train_robbery_target>()->has_tag(tag_type::penalty)) {
            return 10;
        }
        return 0;
    }

    void effect_train_robbery_discard::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        card_ptr target_card = ctx.get<contexts::train_robbery_target>();

        origin->m_game->add_log("LOG_DISCARDED_SELF_CARD", origin, target_card);
        origin->discard_card(target_card);
    }

    struct request_train_robbery_bang : request_bang {
        request_train_robbery_bang(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr target_card)
            : request_bang(origin_card, origin, target, {}, 21)
            , target_card{target_card} {}
        
        card_ptr target_card;

        card_list get_highlights(player_ptr owner) const override {
            return { target_card };
        }
    };

    int effect_train_robbery_bang::evaluate(card_ptr origin_card, player_ptr origin, const effect_context &) const {
        if (origin->is_ghost()) {
            return 5;
        } else if (origin->m_hp <= 1 && count_missed_cards(origin) == 0) {
            return -5;
        }
        return 0;
    }

    void effect_train_robbery_bang::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) const {
        auto req = origin->m_game->top_request<request_train_robbery>();
        card_ptr target_card = ctx.get<contexts::train_robbery_target>();

        origin->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", origin, target_card, 1);
        origin->m_game->queue_request<request_train_robbery_bang>(req->origin_card, req->origin, origin, target_card);
    }
}
