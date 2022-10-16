#include "draw_atend.h"

#include "../../game.h"
#include "../base/effects.h"

namespace banggame {

    void handler_draw_atend::on_play(card *origin_card, player *origin, int amount) {
        if (amount > 0) {
            effect_draw(amount).on_play(origin_card, origin);
        }
    }
}