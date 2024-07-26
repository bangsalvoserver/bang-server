#include "evan_babbit.h"

#include "effects/base/bang.h"

#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {

    game_string effect_evan_babbit::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (auto req = origin->m_game->top_request<request_bang>(origin)) {
            if (req->num_cards_used()) {
                return "ERROR_INVALID_ACTION";
            }
            
            if (!req->flags.check(effect_flag::is_bang)) {
                return "ERROR_INVALID_ACTION";
            }

            return {};
        } else {
            return "ERROR_INVALID_ACTION";
        }
    }

    void effect_evan_babbit::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        auto req = origin->m_game->top_request();
        origin->m_game->add_log("LOG_DEFLECTED_BANG_TO", origin_card, origin, req->origin_card, target);
        req->target = target;
    }
}