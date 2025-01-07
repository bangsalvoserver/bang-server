#include "borderlands.h"

#include "game/game.h"

namespace banggame {

    void effect_borderlands::on_play(card_ptr origin_card, player_ptr origin) {
        while (!origin->empty_hand()) {
            origin->discard_card(origin->m_hand.front());
        }
    }
}