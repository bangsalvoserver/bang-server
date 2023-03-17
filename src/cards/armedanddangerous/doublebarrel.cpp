#include "doublebarrel.h"

#include "game/game.h"
#include "cards/base/bang.h"

namespace banggame {

    void effect_doublebarrel::on_play(card *origin_card, player *origin) {
        origin->m_game->add_listener<event_type::apply_bang_modifier>(origin_card, [=](player *p, request_bang *req) {
            if (p == origin) {
                if (origin->m_game->get_card_sign(req->origin_card).is_diamonds()) {
                    req->unavoidable = true;
                }
                origin->m_game->remove_listeners(origin_card);
            }
        });
    }

    game_string modifier_doublebarrel::on_prompt(card *origin_card, player *origin, card *playing_card) {
        card_sign sign = origin->m_game->get_card_sign(playing_card);
        if (sign && !sign.is_diamonds()) {
            return {"PROMPT_CARD_NO_EFFECT", origin_card};
        } else {
            return {};
        }
    }
}