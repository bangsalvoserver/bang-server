#include "evaded.h"

#include "game/game.h"

namespace banggame {

    void effect_evaded::on_play(card *origin_card, player *origin) {
        card *target_card = origin->m_game->top_request()->origin_card;
        
        effect_missed::on_play(origin_card, origin);

        if (target_card && target_card->deck == card_deck_type::main_deck && target_card->pocket != pocket_type::player_hand) {
            origin->m_game->add_log("LOG_STOLEN_SELF_CARD", origin, target_card);
            origin->add_to_hand(target_card);
        }
    }

}