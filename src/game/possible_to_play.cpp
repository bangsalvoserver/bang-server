#include "possible_to_play.h"

namespace banggame {

    template<typename Rng>
    static bool contains_at_least(Rng &&range, int size) {
        return ranges::distance(ranges::take_view(FWD(range), size)) == size;
    }

    bool is_possible_to_play(player *origin, card *target_card, bool is_response) {
        auto &effects = is_response ? target_card->responses : target_card->effects;
        if (effects.empty()) return false;

        return std::ranges::all_of(effects, [&](const effect_holder &holder) {
            switch (holder.target) {
            case target_type::none:
                return !holder.verify(target_card, origin);
            case target_type::player:
                return contains_at_least(make_player_target_set(origin, target_card, holder), 1);
            case target_type::card:
            case target_type::extra_card:
                return contains_at_least(make_card_target_set(origin, target_card, holder), 1);
            case target_type::cards:
                return contains_at_least(make_card_target_set(origin, target_card, holder), std::max<int>(1, holder.target_value));
            case target_type::select_cubes:
                return origin->count_cubes() >= holder.target_value;
            case target_type::self_cubes:
                return target_card->num_cubes >= holder.target_value;
            case target_type::fanning_targets:
                return std::ranges::any_of(origin->m_game->m_players, [&](player *target) {
                    if (target != origin && origin->m_game->calc_distance(origin, target) <= origin->m_weapon_range + origin->m_range_mod) {
                        if (player *target2 = *std::next(player_iterator(target)); target2 != origin && target2->m_distance_mod == 0) return true;
                        if (player *target2 = *std::prev(player_iterator(target)); target2 != origin && target2->m_distance_mod == 0) return true;
                    }
                    return false;
                });
            default:
                return true;
            }
        });
    }
}