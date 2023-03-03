#include "ironhorse.h"

#include "game/game.h"

#include "cards/base/bang.h"

namespace banggame {

    void equip_ironhorse::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({origin_card, 1}, [=](player *target) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                for (player *p : range_all_players(target)) {
                    origin->m_game->queue_request<request_bang>(origin_card, nullptr, target);
                }
            }
        });
    }
}