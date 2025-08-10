#include "select_cubes_repeat.h"

#include "game/possible_to_play.h"

namespace banggame {

    card_list targeting_select_cubes_repeat::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) {
        auto cubes = cube_slots(origin)
            | rv::for_each([](card_ptr slot) {
                return rv::repeat_n(slot, slot->num_cubes());
            })
            | rn::to_vector;
        rn::shuffle(cubes, origin->m_game->bot_rng);
        
        size_t max_count = cubes.size() / ncubes;
        size_t num_repeats = std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng);
        cubes.resize(ncubes * num_repeats);
        return cubes;
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