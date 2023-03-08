#include "ghost.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_ghost::on_equip(card *target_card, player *target) {
        if (!target->alive()) {
            for (card *c : target->m_characters) {
                target->enable_equip(c);
            }
        }
        target->add_player_flags(player_flags::ghost);
    }
    
    void equip_ghost::on_unequip(card *target_card, player *target) {
        target->remove_player_flags(player_flags::ghost);
        if (!target->alive()) {
            target->m_game->queue_action([=]{
                target->m_game->handle_player_death(nullptr, target, discard_all_reason::discard_ghost);
            }, 1);
        }
    }
}