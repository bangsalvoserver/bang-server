#include "cattle_truck.h"

#include "effects/base/pick.h"

#include "game/game.h"

namespace banggame {

    struct request_cattle_truck : selection_picker {
        request_cattle_truck(card_ptr origin_card, player_ptr target)
            : selection_picker(origin_card, target, target) {}

        void on_update() override {
            if (!live) {
                for (int i=0; i<3 && !target->m_game->m_discards.empty(); ++i) {
                    target->m_game->m_discards.back()->move_to(pocket_type::selection, target, card_visibility::shown);
                }
                if (target->m_game->m_selection.empty()) {
                    target->m_game->pop_request();
                } else if (target->m_game->m_selection.size() == 1) {
                    on_pick(target->m_game->m_selection.front());
                }
            }
        }
        
        void on_pick(card_ptr target_card) {
            target->m_game->pop_request();
            target->add_to_hand(target_card);

            while (!target->m_game->m_selection.empty()) {
                target->m_game->m_selection.front()->move_to(pocket_type::discard_pile);
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_CATTLE_TRUCK", origin_card};
            } else {
                return {"STATUS_CATTLE_TRUCK_OTHER", target, origin_card};
            }
        }
    };

    game_string effect_cattle_truck::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->m_game->m_discards.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }

    void effect_cattle_truck::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_cattle_truck>(origin_card, origin);
    }
}