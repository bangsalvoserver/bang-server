#include "escape.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    bool effect_escape::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request(origin)) {
            return req->flags.check(effect_flag::escapable);
        }
        return false;
    }

    void effect_escape::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
    
    bool effect_escape2::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request(origin)) {
            return req->flags.check(effect_flag::escapable)
                && req->flags.check(effect_flag::single_target)
                && !req->flags.check(effect_flag::multi_target);
        }
        return false;
    }

    void effect_escape2::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->pop_request();
    }
}