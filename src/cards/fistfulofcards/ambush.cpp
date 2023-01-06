#include "ambush.h"

#include "game/game.h"

namespace banggame {

    void equip_ambush::on_enable(card *target_card, player *target) {
        target->m_game->add_game_flags(game_flags::disable_player_distances);
    }

    void equip_ambush::on_disable(card *target_card, player *target) {
        target->m_game->remove_game_flags(game_flags::disable_player_distances);
    }
}