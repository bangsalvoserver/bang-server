#include "backfire.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void effect_backfire::on_play(card *origin_card, player *origin) {
        player *target = origin->m_game->top_request()->origin;
        effect_missed::on_play(origin_card, origin);
        if (target) {
            target->m_game->queue_request<request_bang>(origin_card, origin, target,
                effect_flags::escapable | effect_flags::single_target);
        }
    }
}