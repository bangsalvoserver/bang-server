#include "select_cubes_repeat.h"

#include "game/possible_to_play.h"

namespace banggame {

    card_list targeting_select_cubes_repeat::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        size_t max_count = count_cubes(origin) / ncubes;
        size_t num_repeats = std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng);

        return sample_elements(get_all_cubes(origin), ncubes * num_repeats, origin->m_game->bot_rng);
    }

    game_string targeting_select_cubes_repeat::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const card_list &target_cards) {
        if (target_cards.size() % ncubes != 0) {
            return "ERROR_INVALID_TARGETS";
        }
        for (card_ptr c : target_cards) {
            if (c->owner != origin) {
                return {"ERROR_TARGET_NOT_SELF", origin_card};
            }
        }
        return {};
    }

}