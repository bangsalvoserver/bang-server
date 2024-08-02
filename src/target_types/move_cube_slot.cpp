#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    using visit_cards = play_visitor<"move_cube_slot">;

    static auto make_move_cube_target_set(player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
        return origin->m_table
            | rv::filter(&card::is_orange)
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, max_cubes - slot->num_cubes);
            });
    }

    template<> bool visit_cards::possible(const effect_context &ctx) {
        return origin->first_character()->num_cubes != 0
            && contains_at_least(make_move_cube_target_set(origin, origin_card, ctx), 1);
    }

    template<> card_list visit_cards::random_target(const effect_context &ctx) {
        auto targets = make_move_cube_target_set(origin, origin_card, ctx) | rn::to_vector;
        rn::shuffle(targets, origin->m_game->bot_rng);
        
        targets.resize(std::min({
            static_cast<size_t>(origin->first_character()->num_cubes),
            static_cast<size_t>(effect.target_value),
            targets.size()
        }));
        return targets;
    }

    template<> game_string visit_cards::get_error(const effect_context &ctx, const card_list &targets) {
        if (targets.empty() || targets.size() > effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        origin_card = origin->first_character();
        if (origin_card->num_cubes < targets.size()) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", origin_card};
        }
        for (card_ptr target_card : targets) {
            if (target_card->pocket != pocket_type::player_table || target_card->owner != origin || !target_card->is_orange()) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
            int count = target_card->num_cubes;
            for (card_ptr target : targets) {
                if (target == target_card) ++count;
            }
            if (count > max_cubes) {
                return {"ERROR_CARD_HAS_FULL_CUBES", target_card};
            }
        }
        return {};
    }

    template<> game_string visit_cards::prompt(const effect_context &ctx, const card_list &targets) {
        return {};
    }

    template<> void visit_cards::add_context(effect_context &ctx, const card_list &targets) {}

    template<> void visit_cards::play(const effect_context &ctx, const card_list &targets) {
        if (rn::all_of(targets, [first=targets.front()](card_ptr target_card) { return target_card == first; })) {
            origin->first_character()->move_cubes(targets.front(), static_cast<int>(targets.size()));
        } else {
            for (card_ptr target_card : targets) {
                origin->first_character()->move_cubes(target_card, 1);
            }
        }
    }

}