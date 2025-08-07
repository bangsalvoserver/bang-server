#include "card.h"

#include "cards/game_enums.h"

namespace banggame {

    game_string targeting_card::get_error(const effect_context &ctx, card_ptr target) {
        if (target->owner) {
            MAYBE_RETURN(check_player_filter(origin_card, origin, effect.player_filter, target->owner, ctx));
        }
        MAYBE_RETURN(check_card_filter(origin_card, origin, effect.card_filter, target, ctx));
        return effect.get_error(origin_card, origin, target, ctx);
    }

    prompt_string targeting_card::on_prompt(const effect_context &ctx, card_ptr target) {
        return effect.on_prompt(origin_card, origin, target, ctx);
    }

    void targeting_card::add_context(effect_context &ctx, card_ptr target) {
        ctx.selected_cards.push_back(target);
        effect.add_context(origin_card, origin, target, ctx);
    }

    void targeting_card::on_play(const effect_context &ctx, card_ptr target) {
        effect.on_play(origin_card, origin, target, effect_flag::single_target, ctx);
    }

}