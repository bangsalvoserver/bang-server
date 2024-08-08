#include "a_little_nip.h"

#include "effects/base/heal.h"

#include "game/game.h"

namespace banggame {

    inline effect_heal build_effect_heal(card_ptr origin_card, const effect_context &ctx) {
        return effect_heal{ctx.selected_cubes.count(origin_card) + 1};
    }
    
    game_string effect_a_little_nip::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        return build_effect_heal(origin_card, ctx).on_prompt(origin_card, origin);
    }

    void effect_a_little_nip::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        build_effect_heal(origin_card, ctx).on_play(origin_card, origin);
    }

}