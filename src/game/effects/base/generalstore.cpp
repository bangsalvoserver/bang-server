#include "generalstore.h"

#include "../../game.h"

namespace banggame {

    void request_generalstore::on_pick(pocket_type pocket, player *target_player, card *target_card) {
        target->m_game->pop_request();
        target->m_game->add_log("LOG_DRAWN_FROM_GENERALSTORE", target, target_card, origin_card);
        target->add_to_hand(target_card);
        target->m_game->update_request();
    }

    game_string request_generalstore::status_text(player *owner) const {
        if (target == owner) {
            return {"STATUS_GENERALSTORE", origin_card};
        } else {
            return {"STATUS_GENERALSTORE_OTHER", target, origin_card};
        }
    }

    void effect_generalstore::on_play(card *origin_card, player *origin) {
        for (int i=0; i<origin->m_game->num_alive(); ++i) {
            origin->m_game->draw_card_to(pocket_type::selection);
        }
        for (player &target : range_all_players(origin)) {
            origin->m_game->queue_request<request_generalstore>(origin_card, origin, &target);
        }
    }
}