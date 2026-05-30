#include "scorpion.h"

#include "cards/game_events.h"

#include "effects/base/bang.h"
#include "effects/base/resolve.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct request_scorpion : request_resolvable, interface_missable {
        request_scorpion(card_ptr origin_card, player_ptr origin, player_ptr target, card_ptr target_card)
            : request_resolvable(origin_card, origin, target)
            , target_card{target_card} {}
        
        card_ptr target_card;

        card_list get_highlights(player_ptr owner) const override {
            return {target_card};
        }

        void on_update() override {
            if (target->empty_hand()) {
                auto_resolve();
            }
        }

        void on_resolve() override {
            pop_request();
            origin->discard_card(target_card);
        }

        void on_miss(card_ptr c, effect_flags missed_flags = {}) override {
            pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_RICOCHET", origin_card, target_card};
            } else {
                return {"STATUS_RICOCHET_OTHER", target, origin_card, target_card};
            }
        }
    };

    game_string equip_scorpion::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_self(origin_card, origin, target));
        return {};
    }

    void equip_scorpion::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_equip_card>({target_card, -1}, [=](player_ptr origin, player_ptr e_target, card_ptr origin_card, const effect_context &ctx) {
            if (origin == target && origin_card != target_card && !origin_card->is_black()) {
                target->m_game->queue_request<request_check>(target, target_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
                    if (!result) {
                        target->m_game->queue_request<request_scorpion>(target_card, e_target, origin, origin_card);
                    }
                });
            }
        });
    }
}