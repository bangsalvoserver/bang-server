#include "player.h"

#include "cards/game_enums.h"

#include "game/filters.h"

namespace banggame {

    std::pair<player_ptr, int> targeting_player::random_target(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) const {
        return random_max_element(
            possible_targets(origin_card, origin, effect, ctx),
            [&](player_ptr target) {
                return effect.evaluate(origin_card, origin, target, ctx);
            },
            origin->m_game->bot_rng
        );
    }

    game_string targeting_player::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) const {
        MAYBE_RETURN(check_player_filter(origin_card, origin, player_filter, target, ctx));
        return effect.get_error(origin_card, origin, target, ctx);
    }

    prompt_string targeting_player::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) const {
        return effect.on_prompt(origin_card, origin, target, effect_flag::single_target, ctx);
    }

    void targeting_player::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, player_ptr target) const {
        ctx.add(contexts::selected_player{ target });
        effect.add_context(origin_card, origin, target, ctx);
    }

    void targeting_player::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, player_ptr target) const {
        effect.on_play(origin_card, origin, target, effect_flag::single_target, ctx);
    }

}