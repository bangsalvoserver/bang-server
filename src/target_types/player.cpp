#include "player.h"

#include "game/possible_to_play.h"

#include "cards/filter_enums.h"
#include "cards/game_enums.h"

namespace banggame {

    std::generator<player_ptr> targeting_player::possible_targets(const effect_context &ctx) {
        for (player_ptr target : get_all_player_targets(origin, origin_card, effect, ctx)) {
            co_yield target;
        }
    }

    player_ptr targeting_player::random_target(const effect_context &ctx) {
        return random_element(get_all_player_targets(origin, origin_card, effect, ctx), origin->m_game->bot_rng);
    }

    game_string targeting_player::get_error(const effect_context &ctx, player_ptr target) {
        MAYBE_RETURN(check_player_filter(origin_card, origin, effect.player_filter, target, ctx));
        return effect.get_error(origin_card, origin, target, ctx);
    }

    prompt_string targeting_player::on_prompt(const effect_context &ctx, player_ptr target) {
        return effect.on_prompt(origin_card, origin, target, ctx);
    }

    void targeting_player::add_context(effect_context &ctx, player_ptr target) {
        ctx.selected_players.push_back(target);
        effect.add_context(origin_card, origin, target, ctx);
    }

    void targeting_player::on_play(const effect_context &ctx, player_ptr target) {
        effect.on_play(origin_card, origin, target, effect_flag::single_target, ctx);
    }

}