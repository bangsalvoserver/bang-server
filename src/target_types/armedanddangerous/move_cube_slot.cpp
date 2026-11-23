#include "move_cube_slot.h"

#include "effects/armedanddangerous/ruleset.h"

#include "cards/filter_enums.h"

#include "game/possible_to_play.h"

#include "select_cubes.h"

namespace banggame {

    static auto make_move_cube_target_set(player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
        return origin->m_table
            | rv::filter(&card::is_orange)
            | rv::for_each([](card_ptr slot) {
                int score = slot->has_tag(tag_type::penalty) ? -1 : 0;
                return rv::repeat(scored_cube{slot, score}, max_cubes_per_card - slot->num_cubes());
            });
    }

    bool targeting_move_cube_slot::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        return origin->get_character()->num_cubes() != 0
            && contains_at_least(make_move_cube_target_set(origin, origin_card, ctx), 1);
    }

    scored_target<card_list> targeting_move_cube_slot::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        auto targets = make_move_cube_target_set(origin, origin_card, ctx);
        
        int positive_count = rn::count_if(targets, [](const auto &cube) { return cube.score >= 0; });
        int ncubes = std::min({ origin->get_character()->num_cubes(), max_cubes, std::max(1, positive_count) });

        auto [random_cubes, cube_score] = random_max_elements(targets, ncubes, origin->m_game->bot_rng, &scored_cube::score, &scored_cube::target);

        return { std::move(random_cubes), positive_count >= ncubes ? 0 : -20 };
    }

    game_string targeting_move_cube_slot::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) const {
        if (targets.empty() || targets.size() > max_cubes) {
            return "ERROR_INVALID_TARGETS";
        }
        origin_card = origin->get_character();
        if (origin_card->num_cubes() < targets.size()) {
            return {"ERROR_NOT_ENOUGH_CUBES_ON", origin_card};
        }
        for (card_ptr target_card : targets) {
            if (target_card->pocket != pocket_type::player_table || target_card->owner != origin || !target_card->is_orange()) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
            int count = target_card->num_cubes();
            for (card_ptr target : targets) {
                if (target == target_card) ++count;
            }
            if (count > max_cubes_per_card) {
                return {"ERROR_CARD_HAS_FULL_CUBES", target_card};
            }
        }
        return {};
    }

    void targeting_move_cube_slot::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) const {
        if (rn::all_of(targets, [first=targets.front()](card_ptr target_card) { return target_card == first; })) {
            origin->get_character()->move_cubes(targets.front(), static_cast<int>(targets.size()));
        } else {
            for (card_ptr target_card : targets) {
                origin->get_character()->move_cubes(target_card, 1);
            }
        }
    }

}