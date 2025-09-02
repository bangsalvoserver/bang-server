#include "kit_carlson.h"

#include "cards/game_enums.h"

#include "effects/base/draw.h"
#include "effects/base/resolve.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    struct request_kit_carlson_legend : request_resolvable {
        request_kit_carlson_legend(card_ptr origin_card, player_ptr origin, shared_request_draw &&req_draw)
            : request_resolvable(origin_card, nullptr, origin)
            , req_draw{std::move(req_draw)} {}

        shared_request_draw req_draw;
        
        void on_update() override {
            if (update_count == 0) {
                for (int i = req_draw->num_drawn_cards; i < req_draw->num_cards_to_draw; ++i) {
                    req_draw->phase_one_drawn_card()->move_to(pocket_type::selection, target);
                }
            } else {
                on_resolve();
            }
        }

        resolve_type get_resolve_type() const override {
            return resolve_type::dismiss;
        }

        void on_resolve() override {
            target->m_game->pop_request();

            while (!target->m_game->m_selection.empty()) {
                req_draw->add_to_hand_phase_one(target->m_game->m_selection.front());
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON_LEGEND", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_LEGEND_OTHER", target, origin_card};
            }
        }
    };

    void equip_kit_carlson_legend::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_draw_handlers>(target_card, [=](player_ptr origin, shared_request_draw req_draw) {
            if (origin == target) {
                req_draw->handlers.push_back(target_card);
            }
        });
        
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, card_ptr origin_card, shared_request_draw req_draw) {
            if (origin == target && origin_card == target_card) {
                target->m_game->queue_request<request_kit_carlson_legend>(target_card, target, std::move(req_draw));
            }
        });
    }

    bool effect_kit_carlson_legend_response::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_kit_carlson_legend>(target_is{origin}) != nullptr;
    }
}