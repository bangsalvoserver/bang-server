#include "molly_stark.h"

#include "game/game.h"

namespace banggame {

    void equip_molly_stark::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_discard_hand_card>(origin_card, [=](player *target, card *target_card, bool used) {
            if (origin == target && used && origin->m_game->m_playing != origin) {
                origin->m_game->queue_action([=]{
                    if (origin->alive()) {                        
                        origin_card->flash_card();
                        origin->draw_card(1, origin_card);
                    }
                }, -1);
            }
        });
    }
}