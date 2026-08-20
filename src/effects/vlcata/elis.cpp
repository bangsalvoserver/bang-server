#include "elis.h"
#include "game/game_table.h"

namespace banggame {
    game_string effect_elis::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (target_card->sign.is_spades()) {
            return "ERROR_CARD_IS_SPADES";
        }
        return {};
    }

    void effect_elis::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->discard_card(target_card);
    }
}