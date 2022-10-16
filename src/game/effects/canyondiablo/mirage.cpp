#include "mirage.h"

#include "../../game.h"

namespace banggame {

    bool effect_mirage::can_respond(card *origin_card, player *origin) {
        return origin->m_game->pending_requests();
    }

    void effect_mirage::on_play(card *origin_card, player *origin) {
        if (origin->m_game->top_request().origin() == origin->m_game->m_playing) {
            origin->m_game->add_log("LOG_SKIP_TURN", origin->m_game->m_playing);
            origin->m_game->m_playing->skip_turn();
        }
    }
}