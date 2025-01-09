#include "knife_revolver.h"

#include "game/game_table.h"
#include "effects/base/bang.h"
#include "effects/base/draw_check.h"

namespace banggame {

    void effect_knife_revolver::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, &card_sign::is_jack_to_ace, [=](bool result) {
            if (result) {
                origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
                origin->add_to_hand(origin_card);
            }
        });
    }

}