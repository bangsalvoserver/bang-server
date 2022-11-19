#include "fanning.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    game_string effect_fanning::verify(card *origin_card, player *origin) {
        auto set1 = origin->make_player_target_set(origin_card, effect_holder{
            .player_filter{target_player_filter::reachable | target_player_filter::notself}
        });
        auto set2 = origin->make_player_target_set(origin_card, effect_holder{
            .player_filter{target_player_filter::notself}
        });

        if (std::ranges::none_of(set1, [&](player *player1) {
            return std::ranges::any_of(set2, [&](player *player2) {
                return player1 != player2 && origin->m_game->calc_distance(player1, player2) <= 1;
            });
        })) return "ERROR_TARGET_NOT_IN_RANGE";
        return {};
    }

    game_string handler_fanning::verify(card *origin_card, player *origin, player *player1, player *player2) {
        if (player1 == player2 || origin->m_game->calc_distance(player1, player2) > 1) {
            return "ERROR_TARGET_NOT_IN_RANGE";
        }
        return {};
    }

    void handler_fanning::on_play(card *origin_card, player *origin, player *player1, player *player2) {
        effect_bang{}.on_play(origin_card, origin, player1, effect_flags::escapable);
        effect_bang{}.on_play(origin_card, origin, player2, effect_flags::escapable);
    }
}