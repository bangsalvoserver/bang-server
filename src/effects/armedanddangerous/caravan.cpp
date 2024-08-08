#include "caravan.h"

#include "effects/base/draw.h"

#include "game/game.h"

namespace banggame {
    
    inline effect_draw build_effect_draw(card_ptr origin_card, const effect_context &ctx) {
        return effect_draw{ctx.selected_cubes.count(origin_card) + 2};
    }

    void effect_caravan::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        build_effect_draw(origin_card, ctx).on_play(origin_card, origin);
    }

}