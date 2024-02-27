#include "game/play_verify.h"

namespace banggame {

    using visit_cards = play_visitor<target_type::max_cards>;

    template<> game_string visit_cards::get_error(const effect_context &ctx, const serial::card_list &targets) {
        if (targets.empty() || effect.target_value != 0 && targets.size() > effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card *c : targets) {
            MAYBE_RETURN(play_visitor<target_type::card>{origin, origin_card, effect}.get_error(ctx, c));
        }
        return {};
    }

    template<> duplicate_set visit_cards::duplicates(const serial::card_list &targets) {
        return play_visitor<target_type::cards>{origin, origin_card, effect}.duplicates(targets);
    }

    template<> game_string visit_cards::prompt(const effect_context &ctx, const serial::card_list &targets) {
        return play_visitor<target_type::cards>{origin, origin_card, effect}.prompt(ctx, targets);
    }

    template<> void visit_cards::play(const effect_context &ctx, const serial::card_list &targets) {
        play_visitor<target_type::cards>{origin, origin_card, effect}.play(ctx, targets);
    }

}