#include "game/play_verify.h"

#include "cards/filters.h"

namespace banggame {

    using visit_player = play_visitor<target_type::player>;

    template<> game_string visit_player::get_error(const effect_context &ctx, player *target) {
        MAYBE_RETURN(filters::check_player_filter(origin, effect.player_filter, target, ctx));
        return effect.get_error(origin_card, origin, target, ctx);
    }

    template<> duplicate_set visit_player::duplicates(player *target) {
        return {.players{target}};
    }

    template<> game_string visit_player::prompt(const effect_context &ctx, player *target) {
        return effect.on_prompt(origin_card, origin, target, ctx);
    }

    template<> void visit_player::play(const effect_context &ctx, player *target) {
        auto flags = effect_flags::single_target;
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }
        effect.on_play(origin_card, origin, target, flags, ctx);
    }

}