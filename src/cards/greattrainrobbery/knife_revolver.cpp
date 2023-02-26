#include "knife_revolver.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void effect_knife_revolver::on_play(card *origin_card, player *origin, player *target) {
        origin->m_game->draw_check_then(origin, origin_card, [](card_sign sign) {
            switch (sign.rank) {
            case card_rank::rank_J:
            case card_rank::rank_Q:
            case card_rank::rank_K:
            case card_rank::rank_A:
                return true;
            default:
                return false;
            }
        }, [=](bool result) {
            if (result) {
                origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
                origin->add_to_hand(origin_card);
            }
        });
        origin->m_game->queue_action([=]{
            effect_bang{}.on_play(origin_card, origin, target);
        });
    }

}