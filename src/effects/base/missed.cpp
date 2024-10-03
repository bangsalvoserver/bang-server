#include "missed.h"

#include "game/game.h"
#include "bang.h"

#include "game/possible_to_play.h"
#include "cards/game_enums.h"

namespace banggame {

    int count_missed_cards(player_ptr target) {
        // this doesn't account for calamity janet, elena fuente, caboose
        int count = 0;
        for (card_ptr c : get_all_playable_cards(target, true, effect_context{ .temp_missable = true })) {
            if (c->pocket != pocket_type::button_row && c->pocket != pocket_type::hidden_deck) {
                ++count;
            }
        }
        return count;
    }
    
    bool effect_missed::can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (auto req = origin->m_game->top_request<missable_request>(origin)) {
            return req->can_miss(origin_card);
        }
        return ctx.temp_missable;
    }

    game_string effect_missed::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_bang>(origin)) {
            if (req->bang_strength > 1 && count_missed_cards(origin) < req->bang_strength) {
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

    game_string handler_play_as_missed::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr target_card) {
        if (!effect_missedcard{}.can_play(target_card, origin, ctx)) {
            return {"ERROR_CANT_PLAY_CARD", target_card};
        }
        return {};
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