#include "train_robbery.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "effects/base/bang.h"
#include "effects/base/missed.h"

#include "cards/game_enums.h"
#include "cards/filter_enums.h"

namespace banggame {

    struct request_train_robbery : request_base, interface_target_set_cards, escapable_request {
        using request_base::request_base;

        std::set<const_card_ptr> selected_cards;

        bool can_escape(card_ptr c) const override {
            return selected_cards.empty() && escapable_request::can_escape(c);
        }

        void on_update() override {
            if (!target->alive() || target->immune_to(origin_card, origin, flags)
                || rn::none_of(target->m_table, [&](card_ptr target_card) {
                    return in_target_set(target_card);
                })
            ) {
                target->m_game->pop_request();
            }
        }

        bool in_target_set(const_card_ptr target_card) const override {
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

    game_string effect_train_robbery::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        if (rn::all_of(target->m_table, &card::is_black)) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_train_robbery::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        origin->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        origin->m_game->queue_request<request_train_robbery>(origin_card, origin, target, flags, 20);
    }

    bool effect_train_robbery_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_train_robbery>(target_is{origin}) != nullptr;
    }

    void effect_train_robbery_response::add_context(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_context &ctx) {
        ctx.target_card = target_card;
    }

    void effect_train_robbery_response::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        auto req = origin->m_game->top_request<request_train_robbery>();
        req->selected_cards.insert(target_card);
    }

    game_string effect_train_robbery_discard::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (origin->is_bot()) {
            if (ctx.target_card->has_tag(tag_type::ghost_card)) {
                return "BOT_DISCARD_GHOST";
            }
        }
        return {};
    }

    void effect_train_robbery_discard::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        card_ptr target_card = ctx.target_card;

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

    game_string effect_train_robbery_bang::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (origin->is_bot()) {
            if (!ctx.target_card->self_equippable() && !ctx.target_card->has_tag(tag_type::ghost_card)) {
                return "BOT_DISCARD_GHOST";
            }
            if (origin->m_hp <= 1 && count_missed_cards(origin) == 0) {
                return "BOT_DISCARD_OR_DIE";
            }
        }
        return {};
    }

    void effect_train_robbery_bang::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        auto req = origin->m_game->top_request<request_train_robbery>();
        card_ptr target_card = ctx.target_card;

        origin->m_game->add_log("LOG_RECEIVED_N_BANGS_FOR", origin, target_card, 1);
        origin->m_game->queue_request<request_train_robbery_bang>(req->origin_card, req->origin, origin, target_card);
    }
}