#include "backfire.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    void effect_backfire::on_play(card *origin_card, player *origin) {
        player *target = origin->m_game->top_request()->origin;
        effect_missed::on_play(origin_card, origin);
        if (target) {
            effect_flags flags { effect_flag::escapable, effect_flag::single_target };
            target->m_game->queue_request<request_bang>(origin_card, origin, target, flags, 20);
        }
    }
}