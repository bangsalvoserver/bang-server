#include "ghost.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    inline player_flags flag_for_value(int value) {
        return value == 1 ? player_flags::ghost_1 : player_flags::ghost_2;
    }
    
    void equip_ghost::on_equip(card *target_card, player *target) {
        if (!target->alive()) {
            for (card *c : target->m_characters) {
                target->enable_equip(c);
            }
        }
        target->add_player_flags(flag_for_value(value));
    }
    
    void equip_ghost::on_unequip(card *target_card, player *target) {
        target->remove_player_flags(flag_for_value(value));
        if (!target->alive()) {
            target->m_game->queue_action([=]{
                target->m_game->handle_player_death(nullptr, target, discard_all_reason::discard_ghost);
            }, 1);
        }
    }
}