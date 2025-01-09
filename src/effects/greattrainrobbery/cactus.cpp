#include "cactus.h"

#include "game/game_table.h"
#include "effects/base/draw_check.h"

namespace banggame {

    void effect_cactus::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, &card_sign::is_red, [=](bool result) {
            if (result) {
                origin->heal(1);
            }
        });
    }
}