#include "missed.h"

#include "game/game.h"
#include "bang.h"

#include "game/possible_to_play.h"
#include "cards/game_enums.h"

namespace banggame {
    
    bool effect_missed::can_play(card *origin_card, player *origin) {
        if (auto req = origin->m_game->top_request<missable_request>(origin)) {
            return req->can_miss(origin_card);
        }
        return false;
    }

    game_string effect_missed::on_prompt(card *origin_card, player *origin) {
        if (auto req = origin->m_game->top_request<request_bang>(origin)) {
            if (req->bang_strength > 1 && !contains_at_least(get_all_playable_cards(origin, true)
                | ranges::views::filter([](card *c) { return c->pocket != pocket_type::button_row; }),
                req->bang_strength))
            {
                return {"PROMPT_BANG_STRENGTH", req->bang_strength};
            }
        }
        return {};
    }

    void effect_missed::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<missable_request>();
        req->on_miss(origin_card);
    }

    void effect_missedcard::on_play(card *origin_card, player *origin) {
        auto req = origin->m_game->top_request<missable_request>();
        req->on_miss(origin_card, effect_flags::is_missed);
    }
}