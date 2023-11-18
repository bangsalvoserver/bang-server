#include "kit_carlson.h"

#include "draw.h"

#include "game/game.h"

namespace banggame {
    
    struct request_kit_carlson : selection_picker {
        request_kit_carlson(card *origin_card, player *target, shared_request_draw &&req_draw)
            : selection_picker(origin_card, nullptr, target)
            , req_draw(std::move(req_draw)) {}

        shared_request_draw req_draw;
        
        void on_update() override {
            if (!live) {
                for (int i=0; i<3; ++i) {
                    target->m_game->move_card(req_draw->phase_one_drawn_card(), pocket_type::selection, target);
                }
            }
        }

        void on_pick(card *target_card) override {
            req_draw->add_to_hand_phase_one(target_card);
            if (req_draw->num_drawn_cards >= target->get_cards_to_draw()) {
                target->m_game->pop_request();
                while (!target->m_game->m_selection.empty()) {
                    target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::main_deck, nullptr, card_visibility::hidden);
                }
            }
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    void equip_kit_carlson::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin, shared_request_draw req_draw, bool &handled) {
            if (!handled && origin == target && target->get_cards_to_draw() < 3) {
                target->m_game->queue_request<request_kit_carlson>(target_card, target, std::move(req_draw));
                handled = true;
            }
        });
    }
}