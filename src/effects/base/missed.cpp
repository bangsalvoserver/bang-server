#include "missed.h"

#include "game/game.h"
#include "bang.h"

#include "game/possible_to_play.h"
#include "cards/game_enums.h"

namespace banggame {
    
    bool effect_missed::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<missable_request>(origin)) {
            return req->can_miss(origin_card);
        }
        return false;
    }

    game_string effect_missed::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_bang>(origin)) {
            if (req->bang_strength > 1 && !contains_at_least(get_all_playable_cards(origin, true)
                | rv::filter([](card_ptr c) { return c->pocket != pocket_type::button_row; }),
                req->bang_strength))
            {
                return {"PROMPT_BANG_STRENGTH", req->bang_strength};
            }
        }
        return {};
    }

    void effect_missed::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<missable_request>();
        req->on_miss(origin_card);
    }

    void effect_missedcard::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<missable_request>();
        req->on_miss(origin_card, effect_flag::is_missed);
    }

    bool handler_play_as_missed::can_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return effect_missedcard{}.can_play(target_card, origin);
    }

    game_string handler_play_as_missed::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return effect_missedcard{}.on_prompt(target_card, origin);
    }

    void handler_play_as_missed::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_MISSED", target_card, origin);
        origin->discard_used_card(target_card);
        effect_missedcard{}.on_play(target_card, origin);
    }
}