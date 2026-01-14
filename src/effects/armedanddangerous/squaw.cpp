#include "squaw.h"

#include "effects/base/steal_destroy.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"

#include "target_types/armedanddangerous/select_cubes.h"

namespace banggame {

    game_string effect_squaw::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) {
        if (target_card->owner && ctx.get<contexts::selected_cubes>().count(origin_card) != 0) {
            return effect_steal{}.get_error(origin_card, origin, target_card);
        } else {
            return {};
        }
    }

    prompt_string effect_squaw::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) {
        MAYBE_RETURN(prompts::bot_check_target_card(origin, target_card));
        MAYBE_RETURN(prompts::prompt_target_self_card(origin_card, origin, target_card));
        return {};
    }

    void effect_squaw::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, effect_flags flags, const effect_context &ctx) {
        if (target_card->owner) {
            if (ctx.get<contexts::selected_cubes>().count(origin_card) == 0) {
                effect_destroy{}.on_play(origin_card, origin, target_card, flags);
            } else {
                effect_steal{}.on_play(origin_card, origin, target_card, flags);
            }
        }
    }
}