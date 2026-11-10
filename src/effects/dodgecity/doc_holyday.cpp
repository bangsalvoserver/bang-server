#include "doc_holyday.h"

#include "effects/base/bang.h"
#include "effects/base/steal_destroy.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    prompt_string effect_doc_holyday::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        auto target_cards = ctx.get_all<contexts::selected_card>();
        
        MAYBE_RETURN(prompts::select_prompt_fallback_empty(target_cards | rv::transform([&](card_ptr target_card) {
            return prompts::prompt_target_immunity(target_card, origin, target);
        })));
        
        return effect_bang{}.on_prompt(origin_card, origin, target);
    }

    int effect_doc_holyday::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        return effect_bang{}.evaluate(origin_card, origin, target);
    }

    void effect_doc_holyday::on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) const {
        auto target_cards = ctx.get_all<contexts::selected_card>();

        if (!rn::all_of(target_cards, [&](card_ptr target_card) {
            return target->immune_to(target_card, origin, {}, true);
        })) {
            effect_bang{}.on_play(origin_card, origin, target);
        } else {
            for (card_ptr target_card : target_cards) {
                target->immune_to(target_card, origin, {});
            }
        }
    }
}