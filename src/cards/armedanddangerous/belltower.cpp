#include "belltower.h"

#include "game/game.h"

namespace banggame {

    bool modifier_belltower::valid_with_card(card *origin_card, player *origin, card *playing_card) {
        return std::ranges::any_of(playing_card->get_effect_list(origin->m_game->pending_requests()),
            [](const effect_holder &holder) {
                return bool(holder.player_filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2));
            });
    }

    void modifier_belltower::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.ignore_distances = true;
    }
}