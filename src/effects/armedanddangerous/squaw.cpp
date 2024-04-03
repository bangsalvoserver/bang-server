#include "squaw.h"

#include "effects/base/steal_destroy.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/filters.h"

namespace banggame {

    game_string effect_squaw::get_error(card *origin_card, player *origin, card *target_card, const effect_context &ctx) {
        if (target_card->owner && !filters::get_selected_cubes(origin_card, ctx).empty()) {
            return effect_steal{}.get_error(origin_card, origin, target_card);
        } else {
            return {};
        }
    }

    game_string effect_squaw::on_prompt(card *origin_card, player *origin, card *target_card, const effect_context &ctx) {
        if (target_card->owner == origin) {
            return {"PROMPT_TARGET_SELF", origin_card};
        } else {
            return {};
        }
    }

    void effect_squaw::on_play(card *origin_card, player *origin, card *target_card, const effect_context &ctx) {
        const auto flags = effect_flags::escapable | effect_flags::single_target;
        if (target_card->owner) {
            if (filters::get_selected_cubes(origin_card, ctx).empty()) {
                effect_destroy{}.on_play(origin_card, origin, target_card, flags);
            } else {
                effect_steal{}.on_play(origin_card, origin, target_card, flags);
            }
        }
    }
}