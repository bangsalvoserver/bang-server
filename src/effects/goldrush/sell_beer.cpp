#include "sell_beer.h"

#include "game/game_table.h"
#include "game/play_verify.h"

#include "effects/base/beer.h"

namespace banggame {
    
    void effect_sell_beer::add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) {
        ctx.get<contexts::playing_card>() = target;
    }

    game_string effect_sell_beer::get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) {
        return get_play_card_error(origin, target, ctx);
    }

    void effect_sell_beer::on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card) {
        origin->m_game->add_log("LOG_SOLD_BEER", origin, target_card);
        origin->discard_card(target_card);
        origin->add_gold(1);
        origin->m_game->call_event(event_type::on_play_beer{ origin, true });
    }
}