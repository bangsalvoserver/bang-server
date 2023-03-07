#include "moneybag.h"

#include "cards/effect_context.h"

#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    bool modifier_moneybag::valid_with_modifier(card *origin_card, player *origin, card *playing_card) {
        return playing_card->has_tag(tag_type::card_choice);
    }

    game_string modifier_moneybag::get_error(card *origin_card, player *origin, card *playing_card, const effect_context &ctx) {
        if (origin->m_game->m_discards.empty()) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }

        if (ctx.card_choice && playing_card->get_tag_value(tag_type::card_choice) == ctx.card_choice->get_tag_value(tag_type::card_choice)) {
            return {};
        }

        card *target_card = origin->m_game->m_discards.back();

        if (target_card != playing_card || !playing_card->is_brown()) {
            return "INVALID_MODIFIER_CARD";
        }

        return {};
    }

    void modifier_moneybag::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.disable_banglimit = true;
        if (!origin->m_game->m_discards.empty()) {
            ctx.repeat_card = origin->m_game->m_discards.back();
        }
    }
}