#include "graverobber.h"

#include "game/game.h"
#include "cards/base/generalstore.h"

namespace banggame {

    void effect_graverobber::on_play(card *origin_card, player *origin) {
        if (state == 1) {
            for (int i=0; i<origin->m_game->num_alive(); ++i) {
                if (origin->m_game->m_discards.empty()) {
                    origin->m_game->move_card(origin->m_game->top_of_deck(), pocket_type::selection);
                } else {
                    origin->m_game->move_card(origin->m_game->m_discards.back(), pocket_type::selection);
                }
            }
        } else if (state == 2) {
            for (player *target : range_all_players(origin)) {
                origin->m_game->queue_request<request_generalstore>(origin_card, origin, target);
            }
        }
    }
}