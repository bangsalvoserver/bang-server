#include "select_cubes.h"

#include "game/possible_to_play.h"

#include "effects/armedanddangerous/ruleset.h"

namespace banggame {

    bool targeting_select_cubes::is_possible(const effect_context &ctx) {
        return count_cubes(origin) >= effect.target_value;
    }

    card_list targeting_select_cubes::random_target(const effect_context &ctx) {
        auto cubes = cube_slots(origin)
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes());
            })
            | rn::to_vector;
        rn::shuffle(cubes, origin->m_game->bot_rng);
        
        cubes.resize(effect.target_value);
        return cubes;
    }

    game_string targeting_select_cubes::get_error(const effect_context &ctx, const card_list &target_cards) {
        if (target_cards.size() != effect.target_value) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : target_cards) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        return {};
    }

    prompt_string targeting_select_cubes::on_prompt(const effect_context &ctx, const card_list &target_cards) {
        prompt_string out_prompt;
        origin->m_game->call_event(event_type::get_select_cubes_prompt{ origin, ctx, out_prompt });
        return out_prompt;
    }

    void targeting_select_cubes::add_context(effect_context &ctx, const card_list &target_cards) {
        ctx.selected_cubes.insert(origin_card, target_cards, effect.target_value);
    }

    void targeting_select_cubes::on_play(const effect_context &ctx, const card_list &target_cards) {
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