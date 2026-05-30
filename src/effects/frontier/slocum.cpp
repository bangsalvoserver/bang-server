#include "slocum.h"

#include "cards/game_events.h"

#include "effects/base/bang.h"

#include "game/game_table.h"

namespace banggame {

    void equip_slocum::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_bang_modifier>({ target_card, 35 }, [=](player_ptr origin, shared_request_bang req) {
            if (origin == target) {
                origin->draw_card(1, target_card);
            }
        });

        target->m_game->add_listener<event_type::on_turn_end>({ target_card, 35 }, [=](player_ptr origin, bool skipped) {
            if (origin == target && target->get_bangs_played(true) == 0) {
                origin->discard_card(target_card);
            }
        });
    }

    void equip_slocum::on_disable(card_ptr target_card, player_ptr target) {
        target->m_game->remove_listeners({target_card, 35});
    }
}