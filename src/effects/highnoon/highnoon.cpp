#include "highnoon.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_highnoon::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::pre_turn_start>(target_card, [=](player_ptr p) {
            p->damage(target_card, nullptr, 1);
        });
    }
}