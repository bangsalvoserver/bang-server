#include "lastwill.h"

#include "game/game.h"

#include "effects/base/deathsave.h"
#include "effects/base/resolve.h"
#include "effects/base/prompts.h"

namespace banggame {

    struct request_lastwill : request_resolvable {
        request_lastwill(card_ptr origin_card, player_ptr target)
            : request_resolvable(origin_card, nullptr, target) {}

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        void on_update() override {
            if (target->m_hp > 0) {
                on_resolve();
            } else {
                auto_resolve();
            }
        }
        
        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_LASTWILL", origin_card};
            } else {
                return {"STATUS_LASTWILL_OTHER", origin_card, target};
            }
        }
    };

    game_string equip_lastwill::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(bot_check_target_friend(origin, target));
        if (target->m_role == player_role::sheriff) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void equip_lastwill::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_player_death>({origin_card, -1}, [=](player_ptr target, bool tried_save) {
            if (origin == target) {
                origin->m_game->queue_request<request_lastwill>(origin_card, origin);
            }
        });
    }

    bool effect_lastwill::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_lastwill>(origin) != nullptr;
    }

    game_string handler_lastwill::on_prompt(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
        MAYBE_RETURN(bot_check_target_friend(origin, target));
        return {};
    }

    void handler_lastwill::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards, player_ptr target) {
        origin->m_game->pop_request();
        for (card_ptr chosen_card : target_cards) {
            if (chosen_card->visibility != card_visibility::shown) {
                origin->m_game->add_log(update_target::includes(origin, target), "LOG_GIFTED_CARD", origin, target, chosen_card);
                origin->m_game->add_log(update_target::excludes(origin, target), "LOG_GIFTED_A_CARD", origin, target);
            } else {
                origin->m_game->add_log("LOG_GIFTED_CARD", origin, target, chosen_card);
            }
            target->steal_card(chosen_card);
        }
    }
}