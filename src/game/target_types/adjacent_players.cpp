#include "game/play_verify.h"

#include "cards/filters.h"
#include "cards/filter_enums.h"

namespace banggame {

    using visit_players = play_visitor<target_type::adjacent_players>;

    static auto make_adjacent_players_target_set(player *origin, card *origin_card, const effect_context &ctx) {
        effect_holder effect1 { .player_filter = target_player_filter::notself | target_player_filter::reachable };
        effect_holder effect2 { .player_filter = target_player_filter::notself };
        return make_player_target_set(origin, origin_card, effect1, ctx) | rv::for_each([=](player *target1) {
            return make_player_target_set(origin, origin_card, effect2, ctx) | rv::transform([=](player *target2) {
                return std::pair{target1, target2};
            });
        })
        | rv::filter([=](const auto &targets) {
            auto [target1, target2] = targets;
            return origin->m_game->calc_distance(target1, target2) == 1;
        });
    }

    template<> bool visit_players::possible(const effect_context &ctx) {
        return contains_at_least(make_adjacent_players_target_set(origin, origin_card, ctx), 1);
    }

    template<> serial::player_list visit_players::random_target(const effect_context &ctx) {
        auto targets = make_adjacent_players_target_set(origin, origin_card, ctx) | rn::to_vector;
        auto [target1, target2] = random_element(targets, origin->m_game->rng);
        return {target1, target2};
    }

    template<> game_string visit_players::get_error(const effect_context &ctx, const serial::player_list &targets) {
        if (targets.size() != 2) {
            return "ERROR_INVALID_TARGETS";
        }
        if (targets[0] == targets[1] || origin->m_game->calc_distance(targets[0], targets[1]) != 1) {
            return "ERROR_TARGETS_NOT_ADJACENT";
        }
        for (player *target : targets) {
            MAYBE_RETURN(defer<target_type::player>().get_error(ctx, target));
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
        for (player *target : targets) {
            defer<target_type::player>().play(ctx, target);
        }
    }

}