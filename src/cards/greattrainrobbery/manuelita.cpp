#include "manuelita.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_manuelita::on_enable(card *target_card, player *origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({target_card, 3}, [=](player *target) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                origin->m_game->flash_card(target_card);
                origin->draw_card(2, target_card);
            }
        });
    }
}