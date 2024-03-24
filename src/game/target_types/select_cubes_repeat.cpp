#include "game/play_verify.h"

#include "cards/effect_enums.h"

namespace banggame {

    using visit_cubes = play_visitor<target_type::select_cubes_repeat>;

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const serial::card_list &target_cards) {
        if (target_cards.size() % effect.target_value != 0) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : target_cards) {
            if (c->owner != origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx, const serial::card_list &target_cards) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const serial::card_list &target_cards) {
        play_visitor<target_type::select_cubes>{origin, origin_card, effect}.add_context(ctx, target_cards);
    }

    template<> void visit_cubes::play(const effect_context &ctx, const serial::card_list &target_cards) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}