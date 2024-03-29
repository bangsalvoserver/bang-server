#include "caravan.h"

#include "effects/base/draw.h"
#include "game/filters.h"

#include "game/game.h"

namespace banggame {
    
    inline effect_draw build_effect_draw(card *origin_card, const effect_context &ctx) {
        return effect_draw{static_cast<int>(filters::get_selected_cubes(origin_card, ctx).size()) / 2 + 2};
    }

    void effect_caravan::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        build_effect_draw(origin_card, ctx).on_play(origin_card, origin);
    }

}