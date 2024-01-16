#include "bandolier.h"

#include "game/game.h"

#include "cards/effect_enums.h"

namespace banggame {

    bool modifier_bandolier::valid_with_card(card *origin_card, player *origin, card *playing_card) {
        return rn::contains(playing_card->get_effect_list(origin->m_game->pending_requests()),
                effect_type::banglimit, &effect_holder::type);
    }

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