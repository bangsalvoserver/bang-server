#include "dutch_will.h"

#include "effects/base/draw.h"

#include "game/game.h"

namespace banggame {

    struct request_dutch_will : selection_picker {
        request_dutch_will(card_ptr origin_card, player_ptr target, shared_request_draw &&req_draw)
            : selection_picker(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        shared_request_draw req_draw;
        
        void on_update() override {
            if (!live) {
                int ncards = req_draw->get_cards_to_draw();
                for (int i=0; i<ncards; ++i) {
                    req_draw->phase_one_drawn_card()->move_to(pocket_type::selection, target);
                }
            }
        }

        void on_pick(card_ptr target_card) override {
            req_draw->add_to_hand_phase_one(target_card);
            if (target->m_game->m_selection.size() == 1) {
                target->m_game->pop_request();
                target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target->m_game->m_selection.front());
                target->m_game->m_selection.front()->move_to(pocket_type::discard_pile);
                target->add_gold(1);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (target == owner) {
                return {"STATUS_DUTCH_WILL", origin_card};
            } else {
                return {"STATUS_DUTCH_WILL_OTHER", target, origin_card};
            }
        }
    };

    void equip_dutch_will::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player_ptr origin, shared_request_draw req_draw, bool &handled) {
            if (!handled && origin == target && req_draw->get_cards_to_draw() > 1) {
                target->m_game->queue_request<request_dutch_will>(target_card, target, std::move(req_draw));
                handled = true;
            }
        });
    }
}