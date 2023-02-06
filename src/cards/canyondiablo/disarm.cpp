#include "disarm.h"

#include "game/game.h"

namespace banggame {

    void effect_disarm::on_play(card *origin_card, player *origin) {
        player *shooter = origin->m_game->top_request()->origin;
        effect_missed::on_play(origin_card, origin);
        if (shooter) {
            origin->m_game->queue_action([=]{
                if (!shooter->empty_hand()) {
                    card *hand_card = shooter->random_hand_card();
                    origin->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, shooter, hand_card);
                    shooter->discard_card(hand_card);
                }
            });
        }
    }
}