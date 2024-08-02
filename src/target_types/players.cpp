#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    using visit_players = play_visitor<"players">;

    template<> bool visit_players::possible(const effect_context &ctx) {
        return true;
    }

    static auto get_player_targets_range(const_card_ptr origin_card, player_ptr origin, enums::bitset<target_player_filter> player_filter, const effect_context &ctx) {
        return range_alive_players(origin) | rv::filter([=, &ctx](const_player_ptr target) {
            return target != ctx.skipped_player && !filters::check_player_filter(origin_card, origin, player_filter, target, ctx);
        });
    }

    template<> game_string visit_players::get_error(const effect_context &ctx) {
        for (player_ptr target : get_player_targets_range(origin_card, origin, effect.player_filter, ctx)) {
            MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
        }
        return {};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx) {
        auto targets = get_player_targets_range(origin_card, origin, effect.player_filter, ctx);
        if (targets.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        game_string msg;
        for (player_ptr target : targets) {
            msg = effect.on_prompt(origin_card, origin, target, ctx);
            if (!msg) break;
        }
        return msg;
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
        if (origin_card->is_brown()) {
            flags.add(effect_flag::escapable);
        }

        for (player_ptr target : targets) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}