#include "backfire.h"

#include "../../game.h"
#include "../base/bang.h"

namespace banggame {
    
    bool effect_backfire::can_respond(card *origin_card, player *origin) {
        return origin->m_game->pending_requests();
    }

    void effect_backfire::on_play(card *origin_card, player *origin) {
        if (origin->m_game->top_request().origin()) {
            origin->m_game->queue_request<request_bang>(origin_card, origin, origin->m_game->top_request().origin(),
                effect_flags::escapable | effect_flags::single_target);
        }
    }
}