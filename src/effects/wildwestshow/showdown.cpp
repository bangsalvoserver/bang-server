#include "showdown.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void equip_showdown::on_enable(card *target_card, player *target) {
        target->m_game->add_game_flags(game_flags::treat_any_as_bang);
    }

    void equip_showdown::on_disable(card *target_card, player *target) {
        target->m_game->remove_game_flags(game_flags::treat_any_as_bang);
    }
}