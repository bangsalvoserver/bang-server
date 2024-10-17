#include "discard_black.h"

#include "game/game.h"

#include "effects/base/steal_destroy.h"
#include "effects/base/prompts.h"

#include "cards/filter_enums.h"

namespace banggame {

    game_string effect_discard_black::on_prompt(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        MAYBE_RETURN(bot_check_target_enemy_card(origin, target_card));
        return {};
    }

    game_string effect_discard_black::get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (origin->m_gold < target_card->get_tag_value(tag_type::buy_cost).value_or(0) + 1) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }
        return {};
    }

    void effect_discard_black::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->add_log("LOG_DISCARDED_CARD", origin, target_card->owner, target_card);
        origin->add_gold(-target_card->get_tag_value(tag_type::buy_cost).value_or(0) - 1);
        target_card->owner->discard_card(target_card);
    }
}