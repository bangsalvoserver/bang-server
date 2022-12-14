#include "resolve.h"

#include "game/game.h"

namespace banggame {

    bool effect_resolve::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request_is<resolvable_request>(origin);
    }
    
    void effect_resolve::on_play(card *origin_card, player *origin) {
        origin->m_game->top_request().get<resolvable_request>().on_resolve();
    }
}