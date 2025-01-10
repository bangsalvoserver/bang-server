#include "suzy_lafayette.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    static void check_empty_hand(card_ptr origin_card, player_ptr origin, int ncards) {
        origin->m_game->queue_action([=]{
            if (origin->alive() && origin->m_hand.size() < ncards) {
                origin_card->flash_card();
                origin->draw_card(ncards - origin->m_hand.size(), origin_card);
            }
        });
    }

    void equip_suzy_lafayette::on_enable(card_ptr origin_card, player_ptr origin) {
        if (origin->m_game->m_playing) {
            check_empty_hand(origin_card, origin, ncards);
        }

        origin->m_game->add_listener<event_type::on_discard_hand_card>(origin_card, [=, ncards=ncards](player_ptr target, card_ptr target_card, bool used) {
            if (origin == target) {
                check_empty_hand(origin_card, origin, ncards);
            }
        });
    }
}