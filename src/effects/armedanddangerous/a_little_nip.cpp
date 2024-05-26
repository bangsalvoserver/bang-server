#include "a_little_nip.h"

#include "effects/base/heal.h"

#include "game/game.h"

namespace banggame {

    inline effect_heal build_effect_heal(card *origin_card, const effect_context &ctx, int num_cubes) {
        return effect_heal{ctx.selected_cubes.count(origin_card) / num_cubes + 1};
    }
    
    game_string effect_a_little_nip::on_prompt(card *origin_card, player *origin, const effect_context &ctx) {
        return build_effect_heal(origin_card, ctx, num_cubes).on_prompt(origin_card, origin);
    }

    void effect_a_little_nip::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        build_effect_heal(origin_card, ctx, num_cubes).on_play(origin_card, origin);
    }

}