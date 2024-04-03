#include "duck.h"

#include "game/game.h"
#include "game/filters.h"
#include "effects/base/missed.h"

namespace banggame {

    game_string effect_duck::on_prompt(card *origin_card, player *origin, const effect_context &ctx) {
        if (filters::get_selected_cubes(origin_card, ctx).empty()) {
            return {"PROMPT_NO_REDRAW", origin_card};
        } else {
            return {};
        }
    }

    void effect_duck::on_play(card *origin_card, player *origin, const effect_context &ctx) {
        if (!filters::get_selected_cubes(origin_card, ctx).empty()) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, origin_card);
            origin->m_game->add_short_pause(origin_card);
            origin->add_to_hand(origin_card);
        }
    }
}