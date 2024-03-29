#include "next_stop.h"

#include "game/game.h"

namespace banggame {

    void effect_next_stop::on_play(card *origin_card, player *origin) {
        origin->m_game->advance_train(origin);
    }
}