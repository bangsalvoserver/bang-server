#include "benny_brawler.h"

#include "game/game.h"

namespace banggame {

    void equip_benny_brawler::on_enable(card *target_card, player *target) {
        target->m_game->add_listener<event_type::count_train_equips>(target_card, [=](player *origin, int &num_equip, int &num_advance) {
            if (origin == target) {
                num_equip = 0;
            }
        });
    }
}