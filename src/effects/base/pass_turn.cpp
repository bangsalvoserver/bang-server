#include "pass_turn.h"

#include "game/game.h"

namespace banggame {

    game_string effect_pass_turn::on_prompt(card_ptr origin_card, player_ptr origin) {
        int diff = int(origin->m_hand.size()) - origin->max_cards_end_of_turn();
        if (diff >= 1) {
            return {"PROMPT_PASS_DISCARD", diff};
        }
        return {};
    }

    game_string effect_pass_turn::get_error(card_ptr origin_card, player_ptr origin) {
        game_string out_error;
        origin->m_game->call_event(event_type::check_pass_turn{ origin, out_error });
        return out_error;
    }

    void effect_pass_turn::on_play(card_ptr origin_card, player_ptr origin) {
        origin->pass_turn();
    }
}