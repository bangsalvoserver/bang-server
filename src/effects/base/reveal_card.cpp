#include "reveal_card.h"

#include "cards/game_enums.h"

#include "game/game_table.h"

namespace banggame {

    void effect_reveal_card::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        if (!origin->m_game->check_flags(game_flag::hands_shown)) {
            origin->m_game->add_log("LOG_REVEALED_CARD", origin, target_card);
            target_card->set_visibility(card_visibility::shown);
            target_card->add_short_pause();
            target_card->set_visibility(card_visibility::show_owner, origin);
        }
    }
}