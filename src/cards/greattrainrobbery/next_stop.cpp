#include "next_stop.h"

#include "game/game.h"

namespace banggame {

    void effect_next_stop::on_play(card *origin_card, player *origin) {
        ++origin->m_game->train_position;
        origin->m_game->add_update<game_update_type::move_train>(origin->m_game->train_position);
        origin->m_game->call_event<event_type::on_train_advance>(origin);
    }
}