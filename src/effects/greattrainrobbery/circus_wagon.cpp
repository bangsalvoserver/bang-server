#include "circus_wagon.h"

#include "effects/base/pick.h"

#include "game/game_table.h"
#include "game/bot_suggestion.h"
#include "game/prompts.h"

#include "cards/filter_enums.h"

namespace banggame {

    struct request_discard_table : request_picking {
        using request_picking::request_picking;

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_table && target_card->owner == target && !target_card->is_black();
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->pop_request();
            target->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, target, target_card);
            target->discard_card(target_card);
        }

        prompt_string pick_prompt(card_ptr target_card) const override {
            if (target->is_bot()) {
                MAYBE_RETURN(prompts::bot_check_discard_card(target, target_card));
                if (target_card->has_tag(tag_type::ghost_card)) {
                    return {1, "BOT_DISCARD_GHOST"};
                }
                auto is_penalty_card = [](card_ptr c) { return c->has_tag(tag_type::penalty); };
                if (rn::any_of(target->m_table, is_penalty_card) && !is_penalty_card(target_card)) {
                    return "BOT_DISCARD_PENALTY";
                }
            }
            return {};
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_DISCARD_TABLE", origin_card};
            } else {
                return {"STATUS_DISCARD_TABLE_OTHER", target, origin_card};
            }
        }
    };

    prompt_string effect_circus_wagon::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (origin->is_bot()) {
            if (bot_suggestion::is_target_enemy(origin, target)) {
                if (rn::any_of(target->m_table, [](card_ptr target_card) {
                    return target_card->has_tag(tag_type::jail);
                })) {
                    return {1, "BOT_ENEMY_HAS_JAIL"};
                }
            } else {
                return "BOT_TARGET_ENEMY";
            }
        }
        return {};
    }

    void effect_circus_wagon::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->queue_request<request_discard_table>(origin_card, origin, target);
    }
}