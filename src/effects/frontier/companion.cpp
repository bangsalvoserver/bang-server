#include "companion.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"

#include "ruleset.h"

namespace banggame {

    bool modifier_companion::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
        if (player_ptr tracked_player = get_tracked_player(origin_card)) {
            return tracked_player->alive()
                && playing_card->color == card_color_type::brown
                && playing_card->has_tag(tag_type::ranged_effect);
        }
        return false;
    }

    void modifier_companion::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        if (player_ptr target = get_tracked_player(origin_card)) {
            ctx.add(contexts::distance_start{ target });
        }
    }
}