#include "squaw.h"

#include "effects/base/steal_destroy.h"
#include "effects/base/prompts.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    game_string effect_squaw::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) {
        if (target_card->owner && ctx.selected_cubes.count(origin_card) != 0) {
            return effect_steal{}.get_error(origin_card, origin, target_card);
        } else {
            return {};
        }
    }

    game_string effect_squaw::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) {
        MAYBE_RETURN(bot_check_target_enemy_card(origin, target_card));
        if (target_card->owner == origin) {
            return {"PROMPT_TARGET_SELF", origin_card};
        } else {
            return {};
        }
    }

    void effect_squaw::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) {
        const effect_flags flags { effect_flag::escapable, effect_flag::single_target };
        if (target_card->owner) {
            if (ctx.selected_cubes.count(origin_card) == 0) {
                effect_destroy{}.on_play(origin_card, origin, target_card, flags);
            } else {
                effect_steal{}.on_play(origin_card, origin, target_card, flags);
            }
        }
    }
}