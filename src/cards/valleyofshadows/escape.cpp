#include "escape.h"

#include "game/game.h"

namespace banggame {
    
    bool effect_escape::can_respond(card *origin_card, player *origin) {
        return origin->m_game->pending_requests() && origin->m_game->top_request().target() == origin
            && bool(origin->m_game->top_request().flags() & effect_flags::escapable);
    }

    void effect_escape::on_play(card *origin_card, player *origin) {
        origin->m_game->pop_request();
        origin->m_game->update_request();
    }
}