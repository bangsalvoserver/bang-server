#include "leland.h"

#include "game/game.h"

#include "cards/base/generalstore.h"

namespace banggame {

    void equip_leland::on_enable(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({origin_card, 1}, [=](player *target) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                std::vector<player *> targets = ranges::to<std::vector>(range_all_players(target));
                for (player *p : targets) {
                    origin->m_game->move_card(origin->m_game->top_of_deck(), pocket_type::selection);
                }
                for (player *p : targets) {
                    origin->m_game->queue_request<request_generalstore>(origin_card, nullptr, target);
                }
            }
        });
    }
}