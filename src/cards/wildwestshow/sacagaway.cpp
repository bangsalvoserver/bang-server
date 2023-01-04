#include "sacagaway.h"

#include "game/game.h"

namespace banggame {
    
    void equip_sacagaway::on_enable(card *target_card, player *target) {
        // TODO reveal all hand cards

        target->m_game->add_game_flags(game_flags::hands_shown);
    }

    void equip_sacagaway::on_disable(card *target_card, player *target) {
        // TODO hide all hand cards
        
        target->m_game->remove_game_flags(game_flags::hands_shown);
    }
}