#include "belltower.h"

#include "cards/filters.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_belltower::get_error(card *origin_card, player *origin, card *playing_card) {
        if (std::ranges::none_of(playing_card->get_effect_list(origin->m_game->pending_requests()),
            [](const effect_holder &holder) {
                return bool(holder.player_filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2));
            })
        ) {
            return {"ERROR_NO_RANGED_TARGET", origin_card, playing_card};
        }
        return {};
    }

    void modifier_belltower::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.ignore_distances = true;
    }
}