#include "kit_carlson.h"

#include "draw.h"

#include "game/game_table.h"

namespace banggame {
    
    struct request_kit_carlson : selection_picker {
        request_kit_carlson(card_ptr origin_card, player_ptr target, shared_request_draw &&req_draw)
            : selection_picker(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        shared_request_draw req_draw;
        
        void on_update() override {
            if (update_count == 0) {
                for (int i=0; i<3; ++i) {
                    req_draw->phase_one_drawn_card()->move_to(pocket_type::selection, target);
                }
            }
            auto_pick();
        }

        void on_pick(card_ptr target_card) override {
            req_draw->add_to_hand_phase_one(target_card);
            if (req_draw->num_drawn_cards >= req_draw->num_cards_to_draw) {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->m_selection.front()->move_to(pocket_type::main_deck, nullptr, card_visibility::hidden);
                }
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    void equip_kit_carlson::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::get_draw_handlers>(target_card, [=](player_ptr origin, shared_request_draw req_draw) {
            if (origin == target && req_draw->num_cards_to_draw < 3) {
                req_draw->handlers.push_back(target_card);
            }
        });

        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, card_ptr origin_card, shared_request_draw req_draw) {
            if (origin == target && origin_card == target_card) {
                target->m_game->queue_request<request_kit_carlson>(target_card, target, std::move(req_draw));
            }
        });
    }
}