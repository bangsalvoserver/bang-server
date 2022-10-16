#include "pass_turn.h"

#include "../../game.h"

namespace banggame {

    game_string effect_pass_turn::on_prompt(card *origin_card, player *origin) {
        int diff = static_cast<int>(origin->m_hand.size()) - origin->max_cards_end_of_turn();
        if (diff == 1) {
            return "PROMPT_PASS_DISCARD";
        } else if (diff > 1) {
            return {"PROMPT_PASS_DISCARD_PLURAL", diff};
        }
        return {};
    }

    game_string effect_pass_turn::verify(card *origin_card, player *origin) {
        return origin->m_game->call_event<event_type::verify_pass_turn>(origin, game_string{});
    }

    void effect_pass_turn::on_play(card *origin_card, player *origin) {
        origin->pass_turn();
    }
}