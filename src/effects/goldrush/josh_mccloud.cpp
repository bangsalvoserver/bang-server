#include "josh_mccloud.h"

#include "ruleset.h"

#include "effects/base/card_choice.h"

#include "game/possible_to_play.h"

#include "target_types/base/none.h"

namespace banggame {

    struct request_force_play_card : request_base {
        request_force_play_card(card_ptr origin_card, player_ptr target, card_ptr target_card)
            : request_base(origin_card, nullptr, target)
            , target_card(target_card) {}
        
        card_ptr target_card;

        card_list get_highlights(player_ptr owner) const override {
            return {target_card};
        }

        void on_update() override {
            if (get_all_playable_cards(target, true).empty()) {
                pop_request();
                target_card->add_short_pause();
                target_card->move_to(pocket_type::shop_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
            } else if (target_card->is_black()) {
                if (target_card->equip_effects.empty()) {
                    pop_request();
                    target_card->add_short_pause();
                    target->m_game->add_log("LOG_EQUIPPED_CARD", target_card, target);
                    target->equip_card(target_card);
                }
            } else if (!target_card->modifier
                && rn::all_of(target_card->effects, [](const effect_holder &holder) { return holder.target == TARGET_TYPE(none); })
            ) {
                pop_request();
                target_card->add_short_pause();
                target->m_game->add_log("LOG_PLAYED_CARD", target_card, target);
                target_card->move_to(pocket_type::shop_deck, nullptr, card_visibility::shown, false, pocket_position::begin);
                
                for (const effect_holder &effect : target_card->effects) {
                    targeting_none{}.on_play(target_card, target, effect, {});
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target_card->is_black()) {
                if (owner == target) {
                    return {"STATUS_FORCE_EQUIP_CARD", target_card};
                } else {
                    return {"STATUS_FORCE_EQUIP_CARD_OTHER", target, target_card};
                }
            } else {
                if (owner == target) {
                    return {"STATUS_FORCE_PLAY_CARD", target_card};
                } else {
                    return {"STATUS_FORCE_PLAY_CARD_OTHER", target, target_card};
                }
            }
        }
    };

    bool effect_forced_play::can_play(card_ptr origin_card, player_ptr target) {
        return target->m_game->top_request<request_force_play_card>(target_is{target}) != nullptr;
    }

    void effect_forced_play::on_play(card_ptr origin_card, player_ptr target) {
        auto req = target->m_game->top_request<request_force_play_card>();
        req->pop_request();
    }

    game_string modifier_forced_play::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) {
        if (auto req = origin->m_game->top_request<request_force_play_card>(target_is{origin})) {
            if (card_ptr choice_card = ctx.get<contexts::card_choice>()) {
                target_card = choice_card;
            }
            if (target_card == req->target_card) return {};
        }
        return "INVALID_MODIFIER_CARD";
    }

    void modifier_forced_play::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        auto req = origin->m_game->top_request<request_force_play_card>();
        ctx.set<contexts::forced_play>(req->target_card);
    }

    void effect_josh_mccloud::on_play(card_ptr origin_card, player_ptr target) {
        card_ptr target_card = draw_shop_card(target->m_game);
        target->m_game->queue_request<request_force_play_card>(origin_card, target, target_card);
    }
}