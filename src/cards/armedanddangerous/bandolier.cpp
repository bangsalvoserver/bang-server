#include "bandolier.h"

#include "game/game.h"

#include "cards/effect_context.h"

namespace banggame {

    game_string modifier_bandolier::on_prompt(card *origin_card, player *origin, card *playing_card) {
        if (origin->get_bangs_played() <= 0) {
            return {"PROMPT_NO_BANGS_PLAYED", origin_card};
        } else {
            return {};
        }
    }

    void modifier_bandolier::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
    }
}