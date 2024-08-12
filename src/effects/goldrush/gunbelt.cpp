#include "gunbelt.h"

#include "game/game.h"

namespace banggame {

    void equip_gunbelt::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::apply_maxcards_modifier>({target_card, 20 - ncards}, [=, ncards=ncards](const_player_ptr p, int &value) {
            if (p == target) {
                value = ncards;
            }
        });
    }
}