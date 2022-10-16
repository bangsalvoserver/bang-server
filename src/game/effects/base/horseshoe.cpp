#include "horseshoe.h"

#include "../../game.h"

namespace banggame {
    
    void effect_horseshoe::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_num_checks>(target_card, [=](player *origin, int &num_checks) {
            if (origin == target) {
                ++num_checks;
            }
        });
    }

    struct request_check : selection_picker {
        request_check(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}

        void on_pick(pocket_type pocket, player *target_player, card *target_card) override {
            target->m_game->flash_card(target_card);
            target->m_game->pop_request();
            target->m_game->m_current_check.select(target_card);
            target->m_game->update_request();
        }

        game_string status_text(player *owner) const override {
            if (target == owner) {
                return {"STATUS_CHECK", origin_card};
            } else {
                return {"STATUS_CHECK_OTHER", target, origin_card};
            }
        }
    };

    void effect_horseshoe::queue(card *target_card, player *target) {
        target->m_game->queue_request<request_check>(target_card, target);
    }
}