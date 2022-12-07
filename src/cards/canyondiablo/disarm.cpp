#include "disarm.h"

#include "game/game.h"

namespace banggame {

    bool effect_disarm::can_respond(card *origin_card, player *origin) {
        return origin->m_game->pending_requests();
    }

    void effect_disarm::on_play(card *origin_card, player *origin) {
        player *shooter = origin->m_game->top_request().origin();
        if (shooter && !shooter->empty_hand()) {
            card *hand_card = shooter->random_hand_card();
            origin->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, shooter, hand_card);
            shooter->discard_card(hand_card);
        }
    }
}