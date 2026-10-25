#include "select_cubes.h"

#include "effects/armedanddangerous/ruleset.h"

#include "game/game_table.h"

#include "utils/combinations.h"

namespace banggame {

    std::generator<card_list> targeting_select_cubes::possible_targets(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        return utils::combinations(rn::to<card_list>(get_all_cubes(origin)), ncubes);
    }

    card_list targeting_select_cubes::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        return sample_elements(get_all_cubes(origin), ncubes, origin->m_game->bot_rng);
    }

    game_string targeting_select_cubes::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) const {
        if (target_cards.size() != ncubes) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : target_cards) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        return {};
    }

    prompt_string targeting_select_cubes::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) const {
        return origin->m_game->call_event(event_type::get_select_cubes_prompt{ origin, ctx });
    }

    void targeting_select_cubes::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target_cards) const {
        ctx.add(contexts::selected_cubes{ origin_card, target_cards });
    }

    void targeting_select_cubes::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) const {
        using cube_count_pair = std::pair<card_ptr, int>;
        std::vector<cube_count_pair> card_cube_count;

        for (card_ptr cube : target_cards) {
            auto it = rn::find(card_cube_count, cube, &cube_count_pair::first);
            if (it == card_cube_count.end()) {
                card_cube_count.emplace_back(cube, 1);
            } else {
                ++it->second;
            }
        }

        for (const auto &[cube, ncubes] : card_cube_count) {
            cube->move_cubes(nullptr, ncubes);
        }
    }

}