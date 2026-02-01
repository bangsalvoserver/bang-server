#include "evan_babbit.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "game/game_table.h"
#include "game/prompts.h"
#include "game/bot_suggestion.h"

namespace banggame {

    bool effect_evan_babbit::can_play(card_ptr origin_card, player_ptr origin) {
        if (auto req = origin->m_game->top_request<request_bang>(target_is{origin})) {
            return req->num_cards_used() == 0
                && req->flags.check(effect_flag::is_bang);
        } else {
            return false;
        }
    }

    game_string effect_evan_babbit::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        return {};
    }

    void effect_evan_babbit::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        bot_suggestion::signal_hostile_action(origin, target);
        
        auto req = origin->m_game->top_request();
        origin->m_game->add_log("LOG_DEFLECTED_BANG_TO", origin_card, origin, req->origin_card, target);
        req->target = target;
    }
}