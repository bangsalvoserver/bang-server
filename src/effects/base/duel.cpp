#include "duel.h"

#include "bang.h"
#include "pick.h"

#include "game/game.h"
#include "game/filters.h"
#include "game/prompts.h"

namespace banggame {

    struct request_duel : request_resolvable, interface_picking, respondable_with_bang {
        request_duel(card_ptr origin_card, player_ptr origin, player_ptr target, player_ptr respond_to, effect_flags flags = {})
            : request_resolvable(origin_card, origin, target, flags)
            , respond_to(respond_to) {}

        player_ptr respond_to = nullptr;

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                target->m_game->pop_request();
            } else {
                if (!live) {
                    target->play_sound("duel");
                }
                if (target->empty_hand()) {
                    auto_resolve();
                }
            }
        }

        prompt_string resolve_prompt() const override {
            if (target->is_bot() && target->m_hp <= 1 && rn::any_of(target->m_hand, [&](card_ptr target_card) { return can_pick(target_card); })) {
                return "BOT_BAD_PLAY";
            }
            return {};
        }

        bool can_pick(const_card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && target_card->is_bang_card(target)
                && !target->m_game->is_usage_disabled(target_card);
        }

        void respond_with_bang() override {
            target->m_game->pop_request();
            target->m_game->queue_request<request_duel>(origin_card, origin, respond_to, target);
        }

        void on_pick(card_ptr target_card) override {
            target->m_game->add_log("LOG_RESPONDED_WITH_CARD", target_card, target);
            target->discard_used_card(target_card);
            respond_with_bang();
        }

        void on_resolve() override {
            target->m_game->pop_request();
            target->damage(origin_card, origin, 1);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_DUEL", origin_card};
            } else {
                return {"STATUS_DUEL_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_duel::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }

    void effect_duel::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) {
        target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        target->m_game->queue_request<request_duel>(origin_card, origin, target, origin, flags);
    }
}