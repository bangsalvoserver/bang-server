#include "pass_turn.h"

#include "game/game_table.h"

namespace banggame {

    static prompt_string get_pass_turn_prompt(player_ptr origin) {
        return origin->m_game->call_event(event_type::prompt_pass_turn{ origin });
    }

    prompt_string effect_pass_turn::on_prompt(card_ptr origin_card, player_ptr origin) {
        MAYBE_RETURN(get_pass_turn_prompt(origin));
        int diff = int(origin->m_hand.size()) - origin->max_cards_end_of_turn();
        if (diff >= 1) {
            return {"PROMPT_PASS_DISCARD", diff};
        }
        return {};
    }

    game_string effect_pass_turn::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (origin->is_bot() || ctx.contains<contexts::playing_card>()) {
            return origin->m_game->call_event(event_type::check_pass_turn{ origin });
        }
        return {};
    }

    void effect_pass_turn::on_play(card_ptr origin_card, player_ptr origin) {
        origin->pass_turn();
    }
}
