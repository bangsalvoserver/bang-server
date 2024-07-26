#include "next_stop.h"

#include "game/game.h"

namespace banggame {

    void effect_next_stop::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->advance_train(origin);
    }
}