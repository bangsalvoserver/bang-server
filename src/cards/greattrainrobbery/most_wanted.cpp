#include "most_wanted.h"

#include "game/game.h"

namespace banggame {

    void effect_most_wanted::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->queue_action([=]{
            target->m_game->draw_check_then(target, origin_card, std::not_fn(&card_sign::is_spades), [=](bool result) {
                if (!result) {
                    target->damage(origin_card, origin, 1);
                }
            });
        });
    }
}