#include "tequila_joe.h"

#include "effects/base/beer.h"

#include "game/game.h"

namespace banggame {
    
    void equip_tequila_joe::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_beer_modifier>(target_card, [target](player_ptr origin, int &value) {
            if (target == origin) {
                ++value;
            }
        });
    }
}