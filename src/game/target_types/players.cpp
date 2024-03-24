#include "game/play_verify.h"

#include "cards/filters.h"

namespace banggame {

    using visit_players = play_visitor<target_type::players>;

    template<> game_string visit_players::get_error(const effect_context &ctx) {
        for (player *target : range_all_players(origin)) {
            if (target != ctx.skipped_player && !filters::check_player_filter(origin, effect.player_filter, target, ctx)) {
                MAYBE_RETURN(effect.get_error(origin_card, origin, target, ctx));
            }
        }
        return {};
    }

    template<> game_string visit_players::prompt(const effect_context &ctx) {
        std::vector<player *> targets;
        for (player *target : range_all_players(origin)) {
            if (target != ctx.skipped_player && !filters::check_player_filter(origin, effect.player_filter, target, ctx)) {
                targets.push_back(target);
            }
        }
        if (targets.empty()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        }
        game_string msg;
        for (player *target : targets) {
            msg = effect.on_prompt(origin_card, origin, target, ctx);
            if (!msg) break;
        }
        return msg;
    }

    template<> void visit_players::add_context(effect_context &ctx) {
        for (player *target : range_all_players(origin)) {
            if (target != ctx.skipped_player && !filters::check_player_filter(origin, effect.player_filter, target, ctx)) {
                play_visitor<target_type::player>{origin, origin_card, effect}.add_context(ctx, target);
            }
        }
    }

    template<> void visit_players::play(const effect_context &ctx) {
        std::vector<player *> targets;
        for (player *target : range_all_players(origin)) {
            if (target != ctx.skipped_player && !filters::check_player_filter(origin, effect.player_filter, target, ctx)) {
                targets.push_back(target);
            }
        }

        auto flags = effect_flags::multi_target;
        if (targets.size() == 1) {
            flags |= effect_flags::single_target;
        }
        if (origin_card->is_brown()) {
            flags |= effect_flags::escapable;
        }

        for (player *target : targets) {
            effect.on_play(origin_card, origin, target, flags, ctx);
        }
    }

}