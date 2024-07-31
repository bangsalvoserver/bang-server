#include "manuelita.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "ruleset.h"

namespace banggame {
    
    void equip_manuelita::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_train_advance>({target_card, 2}, [=](player_ptr target, shared_locomotive_context ctx) {
            if (origin->m_game->train_position == origin->m_game->m_stations.size()) {
                origin->m_game->queue_action([=]{
                    target_card->flash_card();
                    origin->draw_card(2, target_card);
                }, -1);
            }
        });
    }
}