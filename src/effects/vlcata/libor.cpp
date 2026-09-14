#include "libor.h"
#include "cards/game_events.h"
#include "effects/base/bang.h"
#include "game/game_table.h"

namespace banggame {
    void equip_libor::on_enable(card_ptr target_card, player_ptr target) {
        auto first_bang = std::make_shared<bool>(true);

        target->m_game->add_listener<event_type::on_turn_start>(target_card,
            [first_bang, target](player_ptr origin) {
                if (origin == target) {
                    *first_bang = true;
                }
            });

        target->m_game->add_listener<event_type::apply_bang_modifier>(target_card,
            [first_bang, target, target_card](player_ptr origin, shared_request_bang req) {
                if (origin == target && target->m_game->m_playing == target && *first_bang) {
                    *first_bang = false;
                    target_card->flash_card();
                    req->bang_damage = 2;
                }
            });
    }
}