#include "play_as_gatling.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"
#include "game/play_verify.h"

namespace banggame {

    game_string handler_play_as_gatling::get_error(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card) {
        return get_play_card_error(origin, chosen_card, ctx);
    }

    game_string handler_play_as_gatling::on_prompt(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card) {
        std::vector<player *> targets;
        for (player *target : range_all_players(origin)) {
            if (target != ctx.skipped_player && target != origin) {
                targets.push_back(target);
            }
        }
        game_string msg;
        for (player *target : targets) {
            if (origin->is_bot() && !bot_suggestion::target_enemy{}.on_check_target(chosen_card, origin, target)) {
                msg = "BOT_BAD_PLAY";
            } else {
                msg = effect_bang{}.on_prompt(chosen_card, origin, target);
            }
            if (!msg) break;
        }
        return msg;
    }

    void handler_play_as_gatling::on_play(card *origin_card, player *origin, const effect_context &ctx, card *chosen_card) {
        origin->m_game->add_log("LOG_PLAYED_CARD_AS_GATLING", chosen_card, origin);
        origin->discard_used_card(chosen_card);

        std::vector<player *> targets;
        for (player *target : range_all_players(origin)) {
            if (target != ctx.skipped_player && target != origin) {
                targets.push_back(target);
            }
        }
        auto flags = effect_flags::play_as_bang | effect_flags::multi_target | effect_flags::skip_target_logs;
        if (targets.size() == 1) {
            flags |= effect_flags::single_target;
        }
        for (player *p : targets) {
            origin->m_game->queue_request<request_bang>(chosen_card, origin, p, flags);
        }
    }

}