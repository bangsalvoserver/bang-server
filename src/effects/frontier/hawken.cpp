#include "hawken.h"

#include "cards/game_enums.h"

#include "effects/base/damage.h"
#include "effects/base/equip.h"
#include "effects/base/can_play_card.h"

#include "game/game_table.h"

namespace banggame {

    struct request_hawken : request_can_play_card {
        using request_can_play_card::request_can_play_card;

        void on_update() override {
            if (!origin->alive()) {
                pop_request();
            } else {
                auto_resolve();
            }
        }
    };

    void equip_hawken::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>({target_card, 3}, [=](card_ptr origin_card, player_ptr origin, player_ptr e_target, int damage, effect_flags flags) {
            if (origin == target && e_target != target && flags.check(effect_flag::is_bang)) {
                target->m_game->queue_request<request_hawken>(target_card, e_target, target, effect_flags{}, 0);
            }
        });
    }

    static bool is_valid_hawken_card(const_card_ptr target_card) {
        return !target_card->is_black()
            && !target_card->is_train()
            && is_self_equippable(target_card);
    }

    bool effect_hawken::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_hawken>(target_is{origin})) {
            return rn::any_of(req->origin->m_table, is_valid_hawken_card);
        }
        return false;
    }

    void effect_hawken::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_hawken>();
        req->pop_request();

        player_ptr target = req->origin;

        card_list target_cards = target->m_table | rv::filter(is_valid_hawken_card) | rn::to<std::vector>();
        for (card_ptr target_card : target_cards) {
            target->m_game->add_log("LOG_STOLEN_SELF_CARD", target, target_card);
            target->steal_card(target_card);
        }
    }
    
}