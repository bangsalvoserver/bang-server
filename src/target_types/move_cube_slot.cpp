#include "game/play_verify.h"

#include "cards/filter_enums.h"

namespace banggame {

    using visit_cards = play_visitor<target_type::move_cube_slot>;

    static auto make_move_cube_target_set(player *origin, card *origin_card, const effect_context &ctx) {
        return origin->m_table
            | rv::filter(&card::is_orange)
            | rv::for_each([](card *slot) {
                return rv::repeat_n(slot, max_cubes - slot->num_cubes);
            });
    }

    template<> bool visit_cards::possible(const effect_context &ctx) {
        return origin->first_character()->num_cubes != 0
            && contains_at_least(make_move_cube_target_set(origin, origin_card, ctx), 1);
    }

    template<> serial::card_list visit_cards::random_target(const effect_context &ctx) {
        auto targets = make_move_cube_target_set(origin, origin_card, ctx) | rn::to_vector;
        size_t num_cubes = std::min<size_t>(origin->first_character()->num_cubes, effect.target_value);
        return targets
            | rv::sample(num_cubes, origin->m_game->rng)
            | rn::to<serial::card_list>;
    }

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
        return {};
    }

    template<> void visit_cards::add_context(effect_context &ctx, const serial::card_list &targets) {}

    template<> void visit_cards::play(const effect_context &ctx, const serial::card_list &targets) {
        for (card *target_card : targets) {
            origin->m_game->move_cubes(origin->first_character(), target_card, 1);
        }
    }

}