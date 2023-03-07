#include "card_choice.h"

#include "cards/filter_enums.h"
#include "cards/effect_context.h"

#include "game/game.h"

namespace banggame {

    bool effect_card_choice::can_play(card *origin_card, player *origin, const effect_context &ctx) {
        return ctx.repeat_card || ctx.card_choice
            && ctx.card_choice->get_tag_value(tag_type::card_choice) == origin_card->get_tag_value(tag_type::card_choice);
    }

    bool modifier_card_choice::valid_with_card(card *origin_card, player *origin, card *target_card) {
        return target_card->pocket == pocket_type::hidden_deck
            && target_card->get_tag_value(tag_type::card_choice) == origin_card->get_tag_value(tag_type::card_choice);
    }

    void modifier_card_choice::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.card_choice = origin_card;
    }
}