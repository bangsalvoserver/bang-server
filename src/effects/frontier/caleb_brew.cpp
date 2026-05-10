#include "caleb_brew.h"

#include "effects/base/heal.h"
#include "effects/base/can_play_card.h"

#include "game/game_table.h"

namespace banggame {

    struct request_caleb_brew : request_can_play_card {
        request_caleb_brew(card_ptr origin_card, player_ptr target, int amount)
            : request_can_play_card(origin_card, nullptr, target)
            , amount{amount} {}
        
        int amount;

        void on_update() override {
            if (amount == 0) {
                pop_request();
            } else {
                auto_resolve();
            }
        }
    };

    void equip_caleb_brew::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_heal>(target_card, [=](card_ptr origin_card, player_ptr origin, player_ptr e_target, bool amount) {
            if (e_target == target && origin_card != target_card) {
                target->m_game->queue_request<request_caleb_brew>(target_card, target, amount);
            }
        });
    }

    bool effect_caleb_brew::can_play(card_ptr origin_card, player_ptr origin) {
        return origin->m_game->top_request<request_caleb_brew>(target_is{origin}) != nullptr;
    }

    void effect_caleb_brew::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<request_caleb_brew>();
        --req->amount;
    }
}