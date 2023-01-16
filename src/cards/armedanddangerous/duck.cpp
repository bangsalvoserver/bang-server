#include "duck.h"

#include "game/game.h"
#include "cards/base/missed.h"

namespace banggame {

    game_string handler_duck::on_prompt(card *origin_card, player *origin, opt_tagged_value<target_type::none> paid_cubes) {
        if (!paid_cubes) {
            return {"PROMPT_NO_REDRAW", origin_card};
        } else {
            return {};
        }
    }

    void handler_duck::on_play(card *origin_card, player *origin, opt_tagged_value<target_type::none> paid_cubes) {
        if (paid_cubes) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
            origin->m_game->add_short_pause(origin_card);
            origin->add_to_hand(origin_card);
        }
    }
}