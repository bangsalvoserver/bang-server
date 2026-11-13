#include "indians.h"

#include "bang.h"
#include "pick.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/filters.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct request_indians : request_auto_resolvable, interface_escapable, interface_picking, respondable_with_bang {
        request_indians(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {})
            : request_auto_resolvable(origin_card, origin, target, flags) {}

        void on_update() override {
            if (target->immune_to(origin_card, origin, flags)) {
                pop_request();
            } else {
                if (update_count == 0) {
                    target->m_game->play_sound(update_target::includes(origin, target), sound_id::indians);
                }
                if (target->empty_hand()) {
                    auto_resolve();
                }
            }
        }

        int evaluate_resolve() const override {
            if (target->is_ghost()) {
                return 5;
            } else if (target->m_hp <= 2) {
                return -5;
            } else {
                return 0;
            }
        }

        bool can_pick(card_ptr target_card) const override {
            return target_card->pocket == pocket_type::player_hand && target_card->owner == target
                && target_card->is_bang_card(target);
        }

        void respond_with_bang() override {
            pop_request();
        }
        
        void on_pick(card_ptr target_card) override {
            target->m_game->add_log("LOG_RESPONDED_WITH_CARD", target_card, target);
            target->discard_used_card(target_card);
            respond_with_bang();
        }

        void on_resolve() override {
            pop_request();
            target->damage(origin_card, origin, 1);
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_INDIANS", origin_card};
            } else {
                return {"STATUS_INDIANS_OTHER", target, origin_card};
            }
        }
    };

    prompt_string effect_indians::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) const {
        MAYBE_RETURN(prompts::prompt_target_immunity(origin_card, origin, target, flags));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }

    int effect_indians::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return bot_suggestion::evaluate_attack(origin, target);
    }
    
    void effect_indians::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags) const {
        bot_suggestion::signal_hostile_action(origin, target, flags);
        
        if (!flags.check(effect_flag::target_players)) {
            target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        }
        target->m_game->queue_request<request_indians>(origin_card, origin, target, flags);
    }
}
