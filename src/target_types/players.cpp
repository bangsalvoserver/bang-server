#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_players = play_visitor<"players">;

    static auto get_player_targets_range(const_card_ptr origin_card, player_ptr origin, enums::bitset<target_player_filter> player_filter, const effect_context &ctx) {
        return origin->m_game->range_alive_players(origin) | rv::filter([=, &ctx](const_player_ptr target) {
            return target != ctx.skipped_player && !check_player_filter(origin_card, origin, player_filter, target, ctx);
        });
    }

    template<> game_string visit_players::get_error(const effect_context &ctx) {
        for (player_ptr target : get_player_targets_range(origin_card, origin, effect.player_filter, ctx)) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    template<> std::generator<std::monostate> visit_players::possible_targets(const effect_context &ctx) {
        if (!get_error(ctx)) {
            co_yield {};
        }
    }

    template<> prompt_string visit_players::prompt(const effect_context &ctx) {
        auto targets = get_player_targets_range(origin_card, origin, effect.player_filter, ctx);
        if (targets.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return merge_prompts(targets | rv::transform([&](player_ptr target) {
            return effect.on_prompt(origin_card, origin, target, ctx);
        }));
    }

    template<> void visit_players::add_context(effect_context &ctx) {
        for (player_ptr target : get_player_targets_range(origin_card, origin, effect.player_filter, ctx)) {
            defer<"player">().add_context(ctx, target);
        }
    }

    template<> void visit_players::play(const effect_context &ctx) {
        auto targets = get_player_targets_range(origin_card, origin, effect.player_filter, ctx);

        effect_flags flags { effect_flag::multi_target, effect_flag::skip_target_logs };
        if (get_single_element(targets)) {
            flags.add(effect_flag::single_target);
        }

        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}