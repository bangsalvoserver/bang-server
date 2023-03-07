#include "express_car.h"

#include "game/game.h"

#include "cards/base/requests.h"

namespace banggame {

    void effect_express_car::on_play(card *origin_card, player *origin) {
        ++origin->m_extra_turns;
        origin->m_game->queue_request<request_discard_hand>(origin_card, origin);
        origin->m_game->queue_action([=]{
            origin->pass_turn();
        });
    }
}