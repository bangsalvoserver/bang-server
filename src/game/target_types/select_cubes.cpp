#include "game/play_verify.h"

#include "cards/effect_enums.h"

namespace banggame {

    using visit_cubes = play_visitor<target_type::select_cubes>;

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const serial::card_list &target_cards) {
        if (target_cards.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : target_cards) {
            if (c->owner != origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> duplicate_set visit_cubes::duplicates(const serial::card_list &target_cards) {
        duplicate_set ret;
        for (card *target : target_cards) {
            ++ret.cubes[target];
        }
        return ret;
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx, const serial::card_list &target_cards) {
        return {};
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const serial::card_list &target_cards) {
        for (card *target : target_cards) {
            effect.add_context(origin_card, origin, target, ctx);
        }
    }

    template<> void visit_cubes::play(const effect_context &ctx, const serial::card_list &target_cards) {
        effect.on_play(origin_card, origin, {}, ctx);
    }

}