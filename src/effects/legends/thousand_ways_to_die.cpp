#include "thousand_ways_to_die.h"

#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    game_string handler_thousand_ways_to_die::get_error(card_ptr origin_card, player_ptr origin, const card_list &target_cards) {
        if (rn::none_of(target_cards, [](card_ptr target_card) { return target_card->has_tag(tag_type::missedcard); })) {
            return "ERROR_NO_MISSED_CARD";
        }
        if (rn::any_of(target_cards, [&](card_ptr target_card) { return target_card->sign.suit != target_cards.front()->sign.suit; })) {
            return "ERROR_DIFFERENT_SUITS";
        }
        return {};
    }

    void handler_thousand_ways_to_die::on_play(card_ptr origin_card, player_ptr origin, const card_list &target_cards) {
        for (card_ptr target_card : target_cards) {
            target_card->set_visibility(card_visibility::shown);
            target_card->add_short_pause();
            target_card->set_visibility(card_visibility::show_owner, origin);
        }
    }
}