#include "bandolier.h"

#include "game/game.h"

#include "cards/filter_enums.h"

namespace banggame {

    bool modifier_bandolier::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr playing_card) {
        return playing_card->has_tag(tag_type::banglimit);
    }

    void modifier_bandolier::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
    }

    game_string effect_bandolier::on_prompt(card_ptr origin_card, player_ptr origin) {
        if (origin->get_bangs_played() <= 0) {
            return {"PROMPT_NO_BANGS_PLAYED", origin_card};
        } else {
            return {};
        }
    }
}