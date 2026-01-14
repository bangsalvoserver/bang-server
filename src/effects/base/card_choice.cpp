#include "card_choice.h"

#include "cards/filter_enums.h"

#include "game/game_table.h"

namespace banggame {

    bool effect_card_choice::can_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx) {
        if (ctx.get<contexts::repeat_card>()) {
            return true;
        }
        if (card_ptr card_choice = ctx.get<contexts::card_choice>()) {
            return card_choice->get_tag_value(tag_type::card_choice) == origin_card->get_tag_value(tag_type::card_choice);
        }
        return false;
    }

    bool modifier_card_choice::valid_with_card(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        return target_card->pocket == pocket_type::hidden_deck
            && target_card->get_tag_value(tag_type::card_choice) == origin_card->get_tag_value(tag_type::card_choice);
    }

    void modifier_card_choice::add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
        ctx.set<contexts::card_choice>(origin_card);
    }
}