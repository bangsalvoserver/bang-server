#include "thousand_ways_to_die.h"

#include "cards/filter_enums.h"

#include "game/game.h"

namespace banggame {

    game_string handler_thousand_ways_to_die::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card1, card_ptr target_card2) {
        if (target_card1 == target_card2) {
            return {"ERROR_DUPLICATE_CARD", target_card2};
        }
        if (target_card1->sign.suit != target_card2->sign.suit) {
            return "ERROR_DIFFERENT_SUITS";
        }
        return {};
    }

    void handler_thousand_ways_to_die::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card1, card_ptr target_card2) {
        target_card1->set_visibility(card_visibility::shown);
        target_card1->add_short_pause();
        target_card1->set_visibility(card_visibility::show_owner, origin);

        target_card2->set_visibility(card_visibility::shown);
        target_card2->add_short_pause();
        target_card2->set_visibility(card_visibility::show_owner, origin);
    }
}