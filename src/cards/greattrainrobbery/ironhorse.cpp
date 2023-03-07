#include "ironhorse.h"

#include "game/game.h"

#include "cards/base/bang.h"

#include "cards/game_enums.h"

namespace banggame {

    void equip_ironhorse::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({origin_card, 1}, [=](player *target, int locomotive_count) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                for (int i=0; i < locomotive_count; ++i) {
                    origin->m_game->queue_action([=]{
                        origin->m_game->add_log("LOG_END_OF_LINE");
                        for (player *p : range_all_players(target)) {
                            origin->m_game->queue_request<request_bang>(origin_card, nullptr, p, effect_flags::multi_target);
                        }
                    }, -1);
                }
            }
        });
    }
}