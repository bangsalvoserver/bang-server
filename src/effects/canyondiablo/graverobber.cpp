#include "graverobber.h"

#include "game/game_table.h"

namespace banggame {

    void effect_graverobber::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        int num_targets = origin->m_game->num_alive() - bool(ctx.skipped_player);
        for (int i=0; i < num_targets; ++i) {
            if (origin->m_game->m_discards.empty()) {
                origin->m_game->top_of_deck()->move_to(pocket_type::selection);
            } else {
                origin->m_game->m_discards.back()->move_to(pocket_type::selection);
            }
        }
    }
}