#include "bullldog.h"

#include "effects/valleyofshadows/play_as_gatling.h"

#include "game/game.h"

namespace banggame {
    
    game_string handler_bulldog::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, card_ptr discarded_card) {
        if (chosen_card == discarded_card) {
            return {"ERROR_DUPLICATE_CARD", discarded_card};
        }
        return handler_play_as_gatling{}.get_error(origin_card, origin, ctx, chosen_card);
    }

    prompt_string handler_bulldog::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, card_ptr discarded_card) {
        return handler_play_as_gatling{}.on_prompt(origin_card, origin, ctx, chosen_card);
    }

    void handler_bulldog::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card, card_ptr discarded_card) {
        handler_play_as_gatling{}.on_play(origin_card, origin, ctx, chosen_card);
    }
}