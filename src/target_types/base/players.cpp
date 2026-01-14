#include "players.h"

#include "game/possible_to_play.h"
#include "game/prompts.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    static auto get_player_targets_range(const_card_ptr origin_card, player_ptr origin, player_filter_bitset player_filter, const effect_context &ctx) {
        player_ptr skipped_player = ctx.get<contexts::skipped_player>();
        return origin->m_game->range_all_players(origin) | rv::filter([=, &ctx](const_player_ptr target) {
            return target != skipped_player && !check_player_filter(origin_card, origin, player_filter, target, ctx);
        });
    }

    game_string targeting_players::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type) {
        for (player_ptr target : get_player_targets_range(origin_card, origin, player_filter, ctx)) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    prompt_string targeting_players::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type) {
        auto targets = get_player_targets_range(origin_card, origin, player_filter, ctx);
        if (targets.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        return prompts::select_prompt_fallback_empty(targets | rv::transform([&](player_ptr target) {
            return effect.on_prompt(origin_card, origin, target, ctx);
        }));
    }

    void targeting_players::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, value_type) {
        for (player_ptr target : get_player_targets_range(origin_card, origin, player_filter, ctx)) {
            effect.add_context(origin_card, origin, target, ctx);
        }
    }

    void targeting_players::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, value_type) {
        auto targets = get_player_targets_range(origin_card, origin, player_filter, ctx);

        effect_flags flags { effect_flag::multi_target, effect_flag::target_players };
        if (get_single_element(targets)) {
            flags.add(effect_flag::single_target);
        }

        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}