#include "belltower.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"

namespace banggame {

    game_string modifier_belltower::get_error(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
        if (rn::none_of(playing_card->get_effect_list(origin->m_game->pending_requests()),
            [](const effect_holder &holder) {
                return holder.player_filter.check(target_player_filter::reachable)
                    || holder.player_filter.check(target_player_filter::range_1)
                    || holder.player_filter.check(target_player_filter::range_2);
            })
        ) {
            return {"ERROR_NO_RANGED_TARGET", origin_card, playing_card};
        }
        return {};
    }

    void modifier_belltower::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.ignore_distances = true;
    }
}