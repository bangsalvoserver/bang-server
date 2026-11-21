#include "select_cubes_repeat.h"

#include "game/possible_to_play.h"

#include "utils/random_element.h"

namespace banggame {

    scored_target<card_list> targeting_select_cubes_repeat::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        auto all_cubes = get_all_cubes(origin, ctx);
        remove_bomb_cube(all_cubes);

        int max_count = static_cast<int>(all_cubes.size()) / ncubes;
        if (max_repeats != 0 && max_count > max_repeats) {
            max_count = max_repeats;
        }
        
        auto [num_repeats, score] = random_max_element(
            rv::iota(0, max_count + 1) | rv::filter([&](int n) {
                return !effect.get_error(origin_card, origin, n, ctx);
            }),
            [&](int n) {
                if (effect.on_prompt(origin_card, origin, n, {}, ctx)) return -2;
                return effect.evaluate(origin_card, origin, n, ctx);
            },
            origin->m_game->bot_rng
        );

        return { sample_elements(all_cubes, ncubes * num_repeats, origin->m_game->bot_rng), score };
    }

    game_string targeting_select_cubes_repeat::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) const {
        if (target_cards.size() % ncubes != 0 || (max_repeats != 0 && target_cards.size() > ncubes * max_repeats)) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : target_cards) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        return effect.get_error(origin_card, origin, target_cards.size() / ncubes, ctx);
    }
    
    prompt_string targeting_select_cubes_repeat::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) const {
        MAYBE_RETURN(targeting_select_cubes::on_prompt(origin_card, origin, effect, ctx, target_cards));
        return effect.on_prompt(origin_card, origin, target_cards.size() / ncubes, {}, ctx);
    }

    void targeting_select_cubes_repeat::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const card_list &target_cards) const {
        targeting_select_cubes::add_context(origin_card, origin, effect, ctx, target_cards);
        effect.add_context(origin_card, origin, target_cards.size() / ncubes, ctx);
    }

    void targeting_select_cubes_repeat::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) const {
        targeting_select_cubes::on_play(origin_card, origin, effect, ctx, target_cards);
        effect.on_play(origin_card, origin, target_cards.size() / ncubes, {}, ctx);
    }

}
