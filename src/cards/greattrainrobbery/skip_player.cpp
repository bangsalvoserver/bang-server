#include "skip_player.h"

#include "game/game.h"

#include "game/filters.h"

namespace banggame {

    game_string modifier_skip_player::get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) {
        if ((playing_card->pocket == pocket_type::player_hand || playing_card->pocket == pocket_type::shop_selection) && !playing_card->is_brown()) {
            return {"ERROR_CANNOT_PLAY_WHILE_EQUIPPING", origin_card};
        } else if (!playing_card->is_modifier()) {
            const auto &effects = playing_card->get_effect_list(origin->m_game->pending_requests());
            if (auto it = std::ranges::find(effects, target_type::players, &effect_holder::target); it != effects.end()) {
                if (ctx.skipped_player && check_player_filter(origin, it->player_filter, ctx.skipped_player, ctx)) {
                    return {"ERROR_CANNOT_SKIP_PLAYER", ctx.skipped_player};
                }
            } else if (auto it = std::ranges::find(effects, target_type::cards_other_players, &effect_holder::target); it != effects.end()) {
                if (ctx.skipped_player == origin) {
                    return {"ERROR_CANNOT_SKIP_PLAYER", origin};
                }
            } else {
                return {"ERROR_NO_PLAYERS_TARGET", origin_card, playing_card};
            }
        }
        return {};
    }

    void modifier_skip_player::add_context(card *origin_card, player *origin, player *target, effect_context &ctx) {
        ctx.skipped_player = target;
    }

}