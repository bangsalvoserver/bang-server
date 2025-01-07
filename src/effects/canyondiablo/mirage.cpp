#include "mirage.h"

#include "game/game.h"

namespace banggame {

    void effect_mirage::on_play(card_ptr origin_card, player_ptr origin) {
        player_ptr target = origin->m_game->top_request()->origin;
        effect_missed::on_play(origin_card, origin);
        if (target && target == origin->m_game->m_playing) {
            target->skip_turn();
        }
    }
}