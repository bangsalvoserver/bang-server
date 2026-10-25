#include "sitej.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    void equip_sitej::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [target, target_card](player_ptr origin, bool skipped) {
            if (!skipped && origin == target) {
                target_card->flash_card();
                if (target->empty_hand()) {
                    target->draw_card(1, target_card);
                } else {
                    card_list hand = target->m_hand;
                    int ncards = static_cast<int>(hand.size());
                    for (card_ptr c : hand) {
                        target->discard_card(c);
                    }
                    target->draw_card(ncards + 1, target_card);
                }
            }
        });
    }

}
