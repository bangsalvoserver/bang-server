#include "cactus.h"

#include "game/game.h"

namespace banggame {

    void effect_cactus::on_play(card *origin_card, player *origin) {
        origin->m_game->draw_check_then(origin, origin_card, &card_sign::is_red, [=](bool result) {
            if (result) {
                origin->heal(1);
            }
        });
    }
}