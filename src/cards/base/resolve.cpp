#include "resolve.h"

#include "game/game.h"

namespace banggame {

    bool effect_resolve::can_respond(card *origin_card, player *origin) {
        return origin->m_game->top_request<resolvable_request>(origin) != nullptr;
    }
    
    void effect_resolve::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<resolvable_request>();
        req->on_resolve();
    }
}