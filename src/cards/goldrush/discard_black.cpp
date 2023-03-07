#include "discard_black.h"

#include "game/game.h"
#include "cards/filter_enums.h"

namespace banggame {

    game_string effect_discard_black::get_error(card *origin_card, player *origin, card *target_card) {
        if (origin->m_gold < target_card->get_tag_value(tag_type::buy_cost).value_or(0) + 1) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }
        return {};
    }

    void effect_discard_black::on_play(card *origin_card, player *origin, card *target_card) {
        origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_card->owner, target_card);
        origin->add_gold(-target_card->get_tag_value(tag_type::buy_cost).value_or(0) - 1);
        target_card->owner->discard_card(target_card);
    }
}