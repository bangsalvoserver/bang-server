#include "josiah_tung.h"

#include "effects/base/can_play_card.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_josiah_tung::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_start>(target_card, [=](player_ptr origin) {
            if (origin == target) {
                origin->m_game->queue_action([=]{
                    origin->m_game->queue_request<request_can_play_card>(target_card, nullptr, origin);
                }, -26);
            }
        });
    }
}