#include "cube_slot.h"

namespace banggame {

    game_string targeting_cube_slot::get_error(const effect_context &ctx, card_ptr target) {
        if (!target->owner) return "ERROR_CARD_HAS_NO_OWNER";

        MAYBE_RETURN(check_player_filter(origin_card, origin, effect.player_filter, target->owner, ctx));

        if (target != target->owner->get_character()
            && !(target->pocket == pocket_type::player_table && target->is_orange())
        ) {
            return "ERROR_TARGET_NOT_CUBE_SLOT";
        }

        if (effect.target_value != 0) {
            if (target == origin->get_character()) {
                return "ERROR_TARGET_PLAYING_CARD";
            }
            if (target->num_cubes() == 0) {
                return {"ERROR_NOT_ENOUGH_CUBES_ON", target};
            }
        }

        return effect.get_error(origin_card, origin, target, ctx);
    }

}