#include "shootout.h"

#include "game/game.h"

#include "effects/base/bang.h"

namespace banggame {

    void equip_shootout::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_bangs_played>({target_card, 1}, [](const_player_ptr p, int &value) {
            --value;
        });
    }
}