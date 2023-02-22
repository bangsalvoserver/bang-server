#include "belltower.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_belltower::get_error(card *origin_card, player *origin, card *playing_card) {
        if (!playing_card->is_modifier()) {
            switch (playing_card->pocket) {
            case pocket_type::player_hand:
            case pocket_type::shop_selection:
                if (!playing_card->is_brown()) {
                    return {"ERROR_CANNOT_PLAY_WHILE_EQUIPPING", origin_card};
                }
                break;
            case pocket_type::main_deck:
            case pocket_type::discard_pile:
            case pocket_type::shop_discard:
                return "ERROR_INVALID_MODIFIER";
            }
            if (std::ranges::none_of(playing_card->get_effect_list(origin->m_game->pending_requests()),
                [](const effect_holder &holder) {
                    return bool(holder.player_filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2));
                })
            ) {
                return {"ERROR_NO_RANGED_TARGET", origin_card, playing_card};
            }
        }
        return {};
    }

    void modifier_belltower::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.ignore_distances = true;
    }
}