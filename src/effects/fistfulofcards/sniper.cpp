#include "sniper.h"

#include "game/game.h"
#include "effects/base/bang.h"

namespace banggame {

    void effect_sniper::on_play(card *origin_card, player *origin, player *target) {
        target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target);
        req->bang_strength = 2;
        target->m_game->queue_request(std::move(req));
    }
}