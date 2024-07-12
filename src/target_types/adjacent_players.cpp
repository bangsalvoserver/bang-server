#include "game/play_verify.h"

#include "game/filters.h"
#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_players = play_visitor<target_type::adjacent_players>;

    static auto make_adjacent_players_target_set(player *origin, card *origin_card, const effect_context &ctx) {
        return rv::cartesian_product(origin->m_game->m_players, origin->m_game->m_players)
            | rv::filter([origin, ignore_distances=ctx.ignore_distances](const auto &pair) {
                auto [target1, target2] = pair;
                return target1 != origin && target2 != origin && target1 != target2
                    && target1->alive() && target2->alive()
                    && (ignore_distances || origin->m_game->calc_distance(origin, target1) <= origin->get_weapon_range() + origin->get_range_mod())
                    && origin->m_game->calc_distance(target1, target2) == 1;
            });
    }

    template<> bool visit_players::possible(const effect_context &ctx) {
        return contains_at_least(make_adjacent_players_target_set(origin, origin_card, ctx), 1);
    }

    template<> serial::player_list visit_players::random_target(const effect_context &ctx) {
        auto targets = make_adjacent_players_target_set(origin, origin_card, ctx) | rn::to_vector;
        auto [target1, target2] = random_element(targets, origin->m_game->bot_rng);
        return {target1, target2};
    }

    template<> game_string visit_players::get_error(const effect_context &ctx, const serial::player_list &targets) {
        if (targets.size() != 2) {
            return "ERROR_INVALID_TARGETS";
        }
        if (!ctx.ignore_distances && origin->m_game->calc_distance(origin, targets[0]) > origin->get_weapon_range() + origin->get_range_mod()) {
            return "ERROR_TARGET_NOT_IN_RANGE";
        }
        if (targets[0] == targets[1] || origin->m_game->calc_distance(targets[0], targets[1]) != 1) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        for (player *target : targets) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx, const serial::player_list &targets) {
        game_string msg;
        for (player *target : targets) {
            msg = defer<target_type::player>().prompt(ctx, target);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_players::add_context(effect_context &ctx, const serial::player_list &targets) {
        for (player *target : targets) {
            defer<target_type::player>().add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx, const serial::player_list &targets) {
        effect_flags flags = effect_flag::multi_target;
        if (origin_card->is_brown()) {
            flags.add(effect_flag::escapable);
        }
        for (player *target : targets) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}