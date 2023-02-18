#include "barrel.h"

#include "game/game.h"
#include "bang.h"

namespace banggame {
    
    void effect_barrel::on_play(card *origin_card, player *target) {
        target->m_game->invoke_action([&]{
            auto req = target->m_game->top_request<missable_request>();
            req->add_card(origin_card);
            target->m_game->draw_check_then(target, origin_card, &card_sign::is_hearts, [=](bool result) {
                if (result) {
                    target->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
                    effect_missed().on_play(origin_card, target);
                }
            });
        });
    }
}