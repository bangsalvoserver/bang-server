#include "kit_carlson.h"

#include "game/game.h"

namespace banggame {
    
    struct request_kit_carlson : selection_picker {
        request_kit_carlson(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}
        
        void on_update() override {
            if (!sent) {
                for (int i=0; i<3; ++i) {
                    target->m_game->move_card(target->m_game->phase_one_drawn_card(), pocket_type::selection, target);
                }
            }
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->add_to_hand_phase_one(target_card);
                if (target->m_num_drawn_cards >= target->get_cards_to_draw()) {
                    target->m_game->pop_request();
                    while (!target->m_game->m_selection.empty()) {
                        target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::main_deck, nullptr, card_visibility::hidden);
                    }
                }
            });
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
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin, bool &override_request) {
            if (!override_request && target == origin && target->get_cards_to_draw() < 3) {
                override_request = true;
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    target->m_game->queue_request<request_kit_carlson>(target_card, target);
                });
            }
        });
    }
}