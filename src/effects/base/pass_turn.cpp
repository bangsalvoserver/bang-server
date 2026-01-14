#include "pass_turn.h"

#include "game/game_table.h"

namespace banggame {

    static prompt_string get_pass_turn_prompt(player_ptr origin) {
        prompt_string result;
        origin->m_game->call_event(event_type::prompt_pass_turn{ origin, result });
        return result;
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
        game_string out_error;
        if (origin->is_bot() || ctx.contains<contexts::playing_card>()) {
            origin->m_game->call_event(event_type::check_pass_turn{ origin, out_error });
        }
        return out_error;
    }

    void effect_pass_turn::on_play(card_ptr origin_card, player_ptr origin) {
        origin->pass_turn();
    }
}
