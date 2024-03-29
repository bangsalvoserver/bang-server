#include "pick.h"

#include "game/game.h"

namespace banggame {

    game_string effect_pick::on_prompt(card *origin_card, player *origin, card *target) {
        return origin->m_game->top_request<request_picking_base>()->pick_prompt(target);
    }

    void effect_pick::on_play(card *origin_card, player *origin, card *target) {
        auto req = origin->m_game->top_request<request_picking_base>();
        req->on_pick(target);
    }

}