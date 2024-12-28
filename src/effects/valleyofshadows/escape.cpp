#include "escape.h"

#include "cards/game_enums.h"

#include "effects/base/resolve.h"

#include "game/game.h"

namespace banggame {
    
    bool effect_escape::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request(origin)) {
            if (auto req2 = std::dynamic_pointer_cast<escapable_request>(req)) {
                return req->origin_card && req->origin_card->is_brown()
                    && !req->flags.check(effect_flag::is_bang)
                    && req2->can_escape(origin_card);
            }
        }
        return false;
    }

    prompt_string effect_escape::on_prompt(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        return req->escape_prompt(origin);
    }

    void effect_escape::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        req->add_card(origin_card);

        origin->m_game->pop_request();
    }
    
    bool effect_escape2::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request(origin)) {
            if (auto req2 = std::dynamic_pointer_cast<escapable_request>(req)) {
                return req->origin_card && req->origin_card->is_brown()
                    && !req->flags.check(effect_flag::is_bang)
                    && req2->can_escape(origin_card)
                    && req->flags.check(effect_flag::single_target)
                    && !req->flags.check(effect_flag::multi_target);
            }
        }
        return false;
    }

    void effect_escape2::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<escapable_request>();
        req->add_card(origin_card);
        
        origin->m_game->pop_request();
    }
}