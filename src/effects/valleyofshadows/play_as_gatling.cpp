#include "play_as_gatling.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/play_verify.h"
#include "game/prompts.h"

#include "utils/range_utils.h"

namespace banggame {

    game_string handler_play_as_gatling::get_error(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card) {
        return get_play_card_error(origin, chosen_card, ctx);
    }
    
    static auto get_player_targets_range(player_ptr origin, const effect_context &ctx) {
        return origin->m_game->range_other_players(origin) | rv::remove(ctx.skipped_player.get());
    }

    game_string handler_play_as_gatling::on_prompt(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card) {
        auto targets = get_player_targets_range(origin, ctx);

        game_string msg;
        for (player_ptr target : targets) {
            msg = effect_bang{}.on_prompt(chosen_card, origin, target);
            if (!msg) break;
        }
        return msg;
    }

    void handler_play_as_gatling::on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx, card_ptr chosen_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_GATLING", chosen_card, origin);
        origin->discard_used_card(chosen_card);

        auto targets = get_player_targets_range(origin, ctx);

        effect_flags flags { effect_flag::play_as_bang, effect_flag::multi_target, effect_flag::skip_target_logs };
        if (get_single_element(targets)) {
            flags.add(effect_flag::single_target);
        }
        for (player_ptr p : targets) {
            origin->m_game->queue_request<request_bang>(chosen_card, origin, p, flags);
        }
    }

}