#include "strongbox.h"

#include "game/game_table.h"

namespace banggame {

    void equip_strongbox::on_enable(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_turn_end>({origin_card, -4}, [=](player_ptr target, bool skipped) {
            if (origin == target && target->alive()) {
                origin_card->flash_card();
                origin->draw_card(1, origin_card);
            }
        });
    }
}