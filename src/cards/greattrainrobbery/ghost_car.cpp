#include "ghost_car.h"

#include "game/game.h"

#include "cards/game_enums.h"

namespace banggame {

    void equip_ghost_car::on_equip(card *origin_card, player *origin) {
        origin->add_player_flags(player_flags::ghost_car);
    }

    void equip_ghost_car::on_unequip(card *origin_card, player *origin) {
        origin->remove_player_flags(player_flags::ghost_car);
        if (!origin->alive()) {
            origin->m_game->queue_action([=]{
                origin->m_game->handle_player_death(nullptr, origin, discard_all_reason::discard_ghost);
            }, 1);
        }
    }
}