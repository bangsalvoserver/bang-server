#include "ranch.h"

#include "effects/base/resolve.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    struct request_ranch : request_resolvable {
        request_ranch(card_ptr target_card, player_ptr target)
            : request_resolvable(target_card, nullptr, target, {}, -25) {}
         
        void on_update() override {
            if (!target->alive() || target->empty_hand() || target->m_game->m_playing != target) {
                target->m_game->pop_request();
            }
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
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
        return origin->m_game->top_request<request_ranch>(target_is{origin}) != nullptr;
    }

    void effect_ranch::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        origin->m_game->pop_request();
        
        origin->draw_card(static_cast<int>(ctx.selected_cards.size()), origin_card);
    }
}