#include "move_cube_slot.h"

#include "game/possible_to_play.h"

#include "cards/filter_enums.h"

namespace banggame {

    static auto make_move_cube_target_set(player_ptr origin, card_ptr origin_card, const effect_context &ctx) {
        return origin->m_table
            | rv::filter(&card::is_orange)
            | rv::for_each([](card_ptr slot) {
                return rv::repeat(slot, max_cubes - slot->num_cubes());
            });
    }

    bool targeting_move_cube_slot::is_possible(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        return origin->get_character()->num_cubes() != 0
            && contains_at_least(make_move_cube_target_set(origin, origin_card, ctx), 1);
    }

    card_list targeting_move_cube_slot::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        return sample_elements(
            make_move_cube_target_set(origin, origin_card, ctx),
            std::min(origin->get_character()->num_cubes(), ncubes),
            origin->m_game->bot_rng
        );
    }

    game_string targeting_move_cube_slot::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        if (targets.empty() || targets.size() > ncubes) {
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
            if (count > max_cubes) {
                return {"ERROR_CARD_HAS_FULL_CUBES", target_card};
            }
        }
        return {};
    }

    void targeting_move_cube_slot::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &targets) {
        if (rn::all_of(targets, [first=targets.front()](card_ptr target_card) { return target_card == first; })) {
            origin->get_character()->move_cubes(targets.front(), static_cast<int>(targets.size()));
        } else {
            for (card_ptr target_card : targets) {
                origin->get_character()->move_cubes(target_card, 1);
            }
        }
    }

}