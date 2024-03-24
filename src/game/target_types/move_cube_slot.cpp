#include "game/play_verify.h"

#include "cards/filter_enums.h"

namespace banggame {

    using visit_cards = play_visitor<target_type::move_cube_slot>;

    template<> game_string visit_cards::get_error(const effect_context &ctx, const serial::card_list &targets) {
        if (targets.empty() || targets.size() > effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        origin_card = origin->first_character();
        if (origin_card->num_cubes < targets.size()) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", origin_card};
        }
        for (card *target_card : targets) {
            if (target_card->pocket != pocket_type::player_table || target_card->owner != origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
            int count = target_card->num_cubes;
            for (card *target : targets) {
                if (target == target_card) ++count;
            }
            if (count > max_cubes) {
                return {"ERROR_CARD_HAS_FULL_CUBES", target_card};
            }
        }
        return {};
    }

    template<> game_string visit_cards::prompt(const effect_context &ctx, const serial::card_list &targets) {
        game_string msg;
        for (card *target : targets) {
            msg = play_visitor<target_type::card>{origin, origin_card, effect}.prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_cards::add_context(effect_context &ctx, const serial::card_list &targets) {
        for (card *c : targets) {
            play_visitor<target_type::card>{origin, origin_card, effect}.add_context(ctx, c);
        }
    }

    template<> void visit_cards::play(const effect_context &ctx, const serial::card_list &targets) {
        for (card *c : targets) {
            play_visitor<target_type::card>{origin, origin_card, effect}.play(ctx, c);
        }
    }

}