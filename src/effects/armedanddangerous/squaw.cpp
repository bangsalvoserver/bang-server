#include "squaw.h"

#include "effects/base/steal_destroy.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

namespace banggame {

    namespace contexts {
        struct squaw_target {
            card_ptr value;
        };
    }

    int effect_squaw::evaluate(card_ptr origin_card, player_ptr origin, card_ptr target_card) const {
        return bot_suggestion::evaluate_target_card(origin, target_card);
    }

    void effect_squaw::add_context(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_context &ctx) const {
        ctx.add(contexts::squaw_target{ target_card });
    }

    game_string effect_squaw::get_error(card_ptr origin_card, player_ptr origin, int ncubes, const effect_context &ctx) const {
        card_ptr target_card = ctx.get<contexts::squaw_target>();
        if (target_card->owner && ncubes != 0) {
            return effect_steal{}.get_error(origin_card, origin, target_card);
        }
        return {};
    }

    prompt_string effect_squaw::on_prompt(card_ptr origin_card, player_ptr origin, int ncubes, const effect_context &ctx) const {
        card_ptr target_card = ctx.get<contexts::squaw_target>();
        if (target_card->owner) {
            if (ncubes != 0) {
                return effect_steal{}.on_prompt(origin_card, origin, target_card, effect_flag::single_target);
            } else {
                return effect_destroy{}.on_prompt(origin_card, origin, target_card, effect_flag::single_target);
            }
        }
        return {};
    }

    int effect_squaw::evaluate(card_ptr origin_card, player_ptr origin, int ncubes) const {
        return ncubes;
    }

    void effect_squaw::on_play(card_ptr origin_card, player_ptr origin, int ncubes, const effect_context &ctx) const {
        card_ptr target_card = ctx.get<contexts::squaw_target>();
        if (target_card->owner) {
            if (ncubes != 0) {
                effect_steal{}.on_play(origin_card, origin, target_card, effect_flag::single_target);
            } else {
                effect_destroy{}.on_play(origin_card, origin, target_card, effect_flag::single_target);
            }
        }
    }
}
