#include "belltower.h"

#include "game/game.h"

namespace banggame {

    game_string modifier_belltower::on_prompt(card *origin_card, player *origin, card *playing_card) {
        if (playing_card->effects.empty() || std::ranges::none_of(playing_card->effects, [](const effect_holder &holder) {
            return bool(holder.player_filter & (target_player_filter::reachable | target_player_filter::range_1 | target_player_filter::range_2));
        })) {
            return {"PROMPT_NO_RANGED_TARGET", origin_card, playing_card};
        } else {
            return {};
        }
    }

    void modifier_belltower::add_context(card *origin_card, player *origin, effect_context &ctx) {
        ctx.ignore_distances = true;
    }
}