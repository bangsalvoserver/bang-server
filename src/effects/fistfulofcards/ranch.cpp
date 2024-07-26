#include "ranch.h"

#include "cards/game_enums.h"

#include "effects/base/steal_destroy.h"
#include "effects/base/resolve.h"

#include "game/game.h"

namespace banggame {

    struct request_ranch : request_resolvable {
        request_ranch(card_ptr target_card, player_ptr target)
            : request_resolvable(target_card, nullptr, target, {}, -8) {}
        
        void on_update() override {
            if (!target->alive() || target->empty_hand() || target->m_game->m_playing != target) {
                target->m_game->pop_request();
            }
        }

        int resolve_type() const override {
            return 1;
        }

        void on_resolve() override {
            target->m_game->pop_request();
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_RANCH", origin_card};
            } else {
                return {"STATUS_RANCH_OTHER", origin_card, target};
            }
        }
    };

    void equip_ranch::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            origin->m_game->queue_request<request_ranch>(target_card, origin);
        });
    }

    bool effect_ranch::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_ranch>(origin) != nullptr;
    }

    void handler_ranch::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards) {
        origin->m_game->pop_request();

        if (!target_cards.empty()) {
            for (card_ptr target : target_cards) {
                effect_discard{}.on_play(origin_card, origin, target);
            }
            origin->draw_card(static_cast<int>(target_cards.size()), origin_card);
        }
    }
}