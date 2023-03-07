#include "cattle_truck.h"

#include "game/game.h"

namespace banggame {

    struct request_cattle_truck : selection_picker {
        request_cattle_truck(card *origin_card, player *target)
            : selection_picker(origin_card, target, target) {}

        void on_update() override {
            if (!sent) {
                for (int i=0; i<3 && !target->m_game->m_discards.empty(); ++i) {
                    target->m_game->move_card(target->m_game->m_discards.back(), pocket_type::selection, target, card_visibility::shown);
                }
                if (target->m_game->m_selection.empty()) {
                    target->m_game->pop_request();
                } else if (target->m_game->m_selection.size() == 1) {
                    on_pick(target->m_game->m_selection.front());
                }
            }
        }
        
        void on_pick(card *target_card) {
            target->m_game->invoke_action([&]{
                target->m_game->pop_request();
                target->add_to_hand(target_card);

                while (!target->m_game->m_selection.empty()) {
                    target->m_game->move_card(target->m_game->m_selection.front(), pocket_type::discard_pile);
                }
            });
        }

        game_string status_text(player *owner) const override {
            if (owner == target) {
                return {"STATUS_CATTLE_TRUCK", origin_card};
            } else {
                return {"STATUS_CATTLE_TRUCK_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_cattle_truck::on_prompt(card *origin_card, player *origin) {
        if (origin->m_game->m_discards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_cattle_truck::on_play(card *origin_card, player *origin) {
        origin->m_game->queue_request<request_cattle_truck>(origin_card, origin);
    }
}