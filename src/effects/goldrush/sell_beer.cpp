#include "sell_beer.h"

#include "game/game.h"

#include "effects/base/beer.h"

namespace banggame {

    void effect_sell_beer::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->add_log("LOG_SOLD_BEER", origin, target_card);
        origin->discard_card(target_card);
        origin->add_gold(1);
        origin->m_game->call_event(event_type::on_play_beer{ origin, true });
    }
}