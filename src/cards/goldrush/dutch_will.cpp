#include "dutch_will.h"

#include "cards/base/draw.h"

#include "game/game.h"

namespace banggame {

    struct request_dutch_will : selection_picker {
        request_dutch_will(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}
        
        void on_update() override {
            if (!sent) {
                int ncards = target->get_cards_to_draw();
                for (int i=0; i<ncards; ++i) {
                    target->m_game->move_card(target->m_game->phase_one_drawn_card(), pocket_type::selection, target);
                }
            }
        }

        void on_pick(card *target_card) override {
            target->m_game->invoke_action([&]{
                target->add_to_hand_phase_one(target_card);
                if (target->m_game->m_selection.size() == 1) {
                    target->m_game->pop_request();
                    target->m_game->add_log("LOG_DISCARDED_SELF_CARD", target, target->m_game->m_selection.front());
                    target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
                    target->add_gold(1);
                }
            });
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_DUTCH_WILL", origin_card};
            } else {
                return {"STATUS_DUTCH_WILL_OTHER", target, origin_card};
            }
        }
    };

    void equip_dutch_will::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::on_draw_from_deck>(target_card, [=](player *origin) {
            if (target->m_game->top_request<request_draw>(target) && origin == target && target->get_cards_to_draw() > 1) {
                target->m_game->invoke_action([&]{
                    target->m_game->pop_request();
                    target->m_game->queue_request<request_dutch_will>(target_card, target);
                });
            }
        });
    }
}