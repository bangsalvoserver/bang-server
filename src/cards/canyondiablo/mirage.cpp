#include "mirage.h"

#include "game/game.h"

namespace banggame {

    void effect_mirage::on_play(card *origin_card, player *origin) {
        player *target = origin->m_game->top_request().origin();
        effect_missed::on_play(origin_card, origin);
        if (target && target == origin->m_game->m_playing) {
            origin->m_game->queue_action([=]{
                target->m_game->add_log("LOG_SKIP_TURN", target);
                target->skip_turn();
            });
        }
    }
}