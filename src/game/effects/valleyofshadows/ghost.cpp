#include "ghost.h"

#include "../../game.h"

namespace banggame {
    
    void effect_ghost::on_equip(card *target_card, player *target) {
        target->add_player_flags(player_flags::targetable);
        for (card *c : target->m_characters) {
            target->enable_equip(c);
        }
    }

    void effect_ghost::on_enable(card *target_card, player *target) {
        target->add_player_flags(player_flags::ghost);
    }

    void effect_ghost::on_disable(card *target_card, player *target) {
        target->remove_player_flags(player_flags::ghost);
    }
    
    void effect_ghost::on_unequip(card *target_card, player *target) {
        target->remove_player_flags(player_flags::targetable);
        target->m_game->queue_action_front([=]{
            target->m_game->handle_player_death(nullptr, target);
        });
    }
}