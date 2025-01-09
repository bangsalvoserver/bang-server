#include "evaded.h"

#include "game/game_table.h"

namespace banggame {

    void effect_evaded::on_play(card_ptr origin_card, player_ptr origin) {
        card_ptr target_card = origin->m_game->top_request()->origin_card;
        
        effect_missed::on_play(origin_card, origin);

        origin->m_game->queue_action([=]{
            if (target_card && target_card->deck == card_deck_type::main_deck && target_card->pocket != pocket_type::player_hand) {
                origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, target_card);
                target_card->add_short_pause();
                origin->add_to_hand(target_card);
            }
        });
    }

}