#include "ghost.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    equip_ghost::equip_ghost(int value)
        : flag(value == 1 ? player_flags::ghost_1 : player_flags::ghost_2) {}
    
    void equip_ghost::on_enable(card *target_card, player *target) {
        if (!target->alive()) {
            for (card *c : target->m_characters) {
                target->enable_equip(c);
            }
        }
        target->add_player_flags(flag);
    }
    
    void equip_ghost::on_disable(card *target_card, player *target) {
        target->remove_player_flags(flag);
        if (!target->alive()) {
            target->m_game->queue_action([=]{
                target->m_game->handle_player_death(nullptr, target, discard_all_reason::discard_ghost);
            }, 2);
        }
    }
}