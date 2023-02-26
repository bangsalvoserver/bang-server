#include "suzy_lafayette.h"

#include "game/game.h"

namespace banggame {

    static void check_empty_hand(card *origin_card, player *origin) {
        origin->m_game->queue_action([=]{
            if (origin->alive() && origin->empty_hand()) {
                origin->m_game->flash_card(origin_card);
                origin->draw_card(1, origin_card);
            }
        }, -1);
    }

    void equip_suzy_lafayette::on_enable(card *origin_card, player *origin) {
        check_empty_hand(origin_card, origin);

        origin->m_game->add_listener<event_type::on_use_hand_card>(origin_card, [=](player *target, card *target_card, bool automatic) {
            if (origin == target) {
                check_empty_hand(origin_card, origin);
            }
        });
    }
}