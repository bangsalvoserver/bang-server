#include "sniper.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

#include "effects/base/bang.h"

namespace banggame {

    game_string effect_sniper::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return prompts::prompt_target_ghost(origin_card, origin, target);
    }

    int effect_sniper::evaluate(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        return bot_suggestion::evaluate_attack(origin, target);
    }

    void effect_sniper::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) const {
        bot_suggestion::signal_hostile_action(origin, target);
        
        target->m_game->add_log("LOG_PLAYED_CARD_ON", origin_card, origin, target);
        auto req = std::make_shared<request_bang>(origin_card, origin, target);
        req->bang_strength = 2;
        target->m_game->queue_request(std::move(req));
    }
}
