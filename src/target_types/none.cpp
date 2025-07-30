#include "none.h"

#include "game/play_verify.h"

namespace banggame {

    game_string targeting_none::get_error(const effect_context &ctx, value_type) {
        return effect.get_error(origin_card, origin, ctx);
    }

    prompt_string targeting_none::on_prompt(const effect_context &ctx, value_type) {
        return effect.on_prompt(origin_card, origin, ctx);
    }

    void targeting_none::add_context(effect_context &ctx, value_type) {
        effect.add_context(origin_card, origin, ctx);
    }

    void targeting_none::on_play(const effect_context &ctx, value_type) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}