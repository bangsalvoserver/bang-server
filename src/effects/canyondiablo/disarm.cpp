#include "disarm.h"

#include "game/game_table.h"

namespace banggame {

    void effect_disarm::on_play(card_ptr origin_card, player_ptr origin) {
        player_ptr shooter = origin->m_game->top_request()->origin;
        effect_missed::on_play(origin_card, origin);
        if (shooter) {
            origin->m_game->queue_action([=]{
                if (!shooter->empty_hand()) {
                    card_ptr hand_card = shooter->random_hand_card();
                    origin->m_game->add_log("LOG_DISCARDED_CARD_FOR", origin_card, shooter, hand_card);
                    shooter->discard_card(hand_card);
                }
            });
        }
    }
}