#include "missed.h"

#include "game/game_table.h"
#include "bang.h"

#include "game/possible_to_play.h"
#include "cards/game_enums.h"

namespace banggame {

    int count_missed_cards(player_ptr target) {
        // this doesn't account for calamity janet, elena fuente, caboose, etc.
        if (!target->m_game->top_request<missable_request>(target_is{target})) {
            throw game_error("invalid call to count_missed_cards()");
        }

        int count = 0;
        for (card_ptr c : get_all_playable_cards(target, true, effect_context{ .temp_missable = true })) {
            ++count;
        }
        return count;
    }
    
    bool effect_missed::can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (auto req = origin->m_game->top_request<missable_request>(target_is{origin})) {
            return req->can_miss(origin_card, ctx);
        }
        return false;
    }

    game_string effect_missed::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_bang>(target_is{origin})) {
            if (req->bang_strength > 1 && count_missed_cards(origin) < req->bang_strength) {
                return {"PROMPT_BANG_STRENGTH", req->bang_strength};
            }
        }
        return {};
    }

    void effect_missed::on_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<missable_request>()) {
            req->on_miss(origin_card);
        } else {
            throw game_error("invalid request access in effect_missed: top_request is not missable_request");
        }
    }

    void effect_missedcard::on_play(card_ptr origin_card, player_ptr origin) {
        auto req = origin->m_game->top_request<missable_request>();
        req->on_miss(origin_card, effect_flag::is_missed);
    }

    bool effect_play_as_missed::can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        return effect_missedcard{}.can_play(ctx.playing_card, origin, ctx);
    }

    game_string effect_play_as_missed::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        return effect_missedcard{}.on_prompt(ctx.playing_card, origin);
    }

    void effect_play_as_missed::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        effect_missedcard{}.on_play(ctx.playing_card, origin);
    }
}