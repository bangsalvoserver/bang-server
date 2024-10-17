#include "sniper.h"

#include "game/game.h"
#include "game/prompts.h"

#include "effects/base/bang.h"

namespace banggame {

    game_string effect_sniper::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }

    void effect_sniper::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target);
        req->bang_strength = 2;
        target->m_game->queue_request(std::move(req));
    }
}