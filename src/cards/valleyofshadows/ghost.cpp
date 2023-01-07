#include "ghost.h"

#include "game/game.h"

namespace banggame {
    
    void equip_ghost::on_equip(card *target_card, player *target) {
        target->add_player_flags(player_flags::ghost);
        for (card *c : target->m_characters) {
            target->enable_equip(c);
        }
    }
    
    void equip_ghost::on_unequip(card *target_card, player *target) {
        target->remove_player_flags(player_flags::ghost);
        target->m_game->queue_action([=]{
            target->m_game->handle_player_death(nullptr, target, discard_all_reason::discard_ghost);
        }, 1);
    }
}