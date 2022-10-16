#include "missed.h"

#include "../../game.h"
#include "bang.h"

namespace banggame {
    
    bool effect_missed_base::can_respond(card *origin_card, player *origin) {
        if (auto *req = origin->m_game->top_request_if<missable_request>(origin)) {
            return req->can_miss(origin_card);
        }
        return false;
    }

    game_string effect_missed_base::on_prompt(card *origin_card, player *origin) {
        if (auto *req = origin->m_game->top_request_if<request_bang>(origin)) {
            if (origin->m_game->make_request_update(origin).respond_cards.size() <= + req->bang_strength) {
                return {"PROMPT_BANG_STRENGTH", req->bang_strength};
            }
        }
        return {};
    }

    void effect_missed::on_play(card *origin_card, player *origin) {
        origin->m_game->top_request().get<missable_request>().on_miss();
    }
}