#include "graverobber.h"

#include "../../game.h"
#include "../base/generalstore.h"

namespace banggame {

    void effect_graverobber::on_play(card *origin_card, player *origin) {
        for (int i=0; i<origin->m_game->num_alive(); ++i) {
            if (origin->m_game->m_discards.empty()) {
                origin->m_game->draw_card_to(pocket_type::selection);
            } else {
                origin->m_game->move_card(origin->m_game->m_discards.back(), pocket_type::selection);
            }
        }
        origin->m_game->queue_request<request_generalstore>(origin_card, origin, origin);
    }
}