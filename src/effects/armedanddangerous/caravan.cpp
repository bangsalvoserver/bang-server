#include "caravan.h"

#include "effects/base/draw.h"

#include "game/game_table.h"

#include "target_types/armedanddangerous/select_cubes.h"

namespace banggame {
    
    inline effect_draw build_effect_draw(card_ptr origin_card, const effect_context &ctx) {
        return effect_draw{contexts::selected_cubes::count_repeats(ctx, origin_card) + 2};
    }

    void effect_caravan::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        build_effect_draw(origin_card, ctx).on_play(origin_card, origin);
    }

}