#include "game/possible_to_play.h"

namespace banggame {

    using visit_cubes = play_visitor<"select_cubes_players">;

    template<> bool visit_cubes::possible(const effect_context &ctx) {
        return true;
    }

    template<> serial::card_list visit_cubes::random_target(const effect_context &ctx) {
        auto cubes = origin->cube_slots()
            | rv::for_each([](card *slot) {
                return rv::repeat_n(slot, slot->num_cubes);
            })
            | rn::to<serial::card_list>;
        rn::shuffle(cubes, origin->m_game->bot_rng);
        
        size_t num_players = rn::distance(get_all_player_targets(origin, origin_card, effect, ctx));
        size_t max_count = std::min(cubes.size(), num_players - 1);
        cubes.resize(std::uniform_int_distribution<size_t>{0, max_count}(origin->m_game->bot_rng));
        return cubes;
    }

    template<> game_string visit_cubes::get_error(const effect_context &ctx, const serial::card_list &target_cards) {
        for (card *c : target_cards) {
            if (c->owner != origin) {
                return "ERROR_TARGET_NOT_SELF";
            }
        }
        return {};
    }

    template<> game_string visit_cubes::prompt(const effect_context &ctx, const serial::card_list &target_cards) {
        return defer<"select_cubes">().prompt(ctx, target_cards);
    }

    template<> void visit_cubes::add_context(effect_context &ctx, const serial::card_list &target_cards) {
        defer<"select_cubes">().add_context(ctx, target_cards);
    }

    template<> void visit_cubes::play(const effect_context &ctx, const serial::card_list &target_cards) {
        defer<"select_cubes">().play(ctx, target_cards);
    }

}