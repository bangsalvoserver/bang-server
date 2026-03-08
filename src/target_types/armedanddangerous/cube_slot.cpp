#include "cube_slot.h"

#include "cards/game_enums.h"

namespace banggame {

    game_string targeting_cube_slot::get_error(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
        if (!target->owner) return "ERROR_CARD_HAS_NO_OWNER";

        MAYBE_RETURN(check_player_filter(origin_card, origin, player_filter, target->owner, ctx));

        if (target != target->owner->get_character()
            && !(target->pocket == pocket_type::player_table && target->is_orange())
        ) {
            return "ERROR_TARGET_NOT_CUBE_SLOT";
        }

        if (stealing) {
            if (target == origin->get_character()) {
                return "ERROR_TARGET_PLAYING_CARD";
            }
            if (target->num_cubes() == 0) {
                return {"ERROR_NOT_ENOUGH_CUBES_ON", target};
            }
        }

        return effect.get_error(origin_card, origin, target, ctx);
    }

    prompt_string targeting_cube_slot::on_prompt(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
        return effect.on_prompt(origin_card, origin, target, ctx);
    }

    void targeting_cube_slot::add_context(card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, card_ptr target) {
        ctx.add<contexts::selected_cards>().push_back(target);
        effect.add_context(origin_card, origin, target, ctx);
    }

    void targeting_cube_slot::on_play(card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, card_ptr target) {
        effect.on_play(origin_card, origin, target, effect_flag::single_target, ctx);
    }

}