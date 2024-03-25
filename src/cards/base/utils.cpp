#include "utils.h"

#include "game/game.h"

namespace banggame {

    bool effect_human::can_play(card *origin_card, player *origin) {
        return !origin->is_bot();
    }

    void effect_set_playing::add_context(card *origin_card, player *origin, card *target, effect_context &ctx) {
        ctx.playing_card = target;
    }

}