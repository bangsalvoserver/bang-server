// cyril.cpp
#include "cyril.h"
#include "effects/base/bang.h"
#include "game/game_table.h"

namespace banggame {
    void equip_cyril::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_bang_modifier>(target_card,
            [=](player_ptr origin, shared_request_bang req) {
                if (req->target == target) {
                    target_card->flash_card();
                    target->draw_card(1, target_card);
                }
            });
    }
}