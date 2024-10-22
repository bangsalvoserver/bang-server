#include "evelyn_shebang.h"

#include "game/game.h"
#include "game/prompts.h"

#include "effects/base/bang.h"

namespace banggame {

    game_string effect_evelyn_shebang::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        game_string out_error;
        origin->m_game->call_event(event_type::check_bang_target{ origin_card, origin, target, effect_flags{}, out_error });
        return out_error;
    }

    game_string effect_evelyn_shebang::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        MAYBE_RETURN(prompts::bot_check_target_enemy(origin, target));
        MAYBE_RETURN(prompts::prompt_target_ghost(origin_card, origin, target));
        return {};
    }

    void effect_evelyn_shebang::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->add_listener<event_type::check_bang_target>(origin_card, [=](card_ptr e_origin_card, player_ptr e_origin, player_ptr e_target, effect_flags flags, game_string &out_error) {
            if (e_origin_card == origin_card && e_origin == origin && e_target == target) {
                out_error = "ERROR_TARGET_NOT_UNIQUE";
            }
        });

        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player_ptr e_origin, bool skipped) {
            if (origin == e_origin) {
                origin->m_game->remove_listeners(origin_card);
            }
        });

        effect_bang{}.on_play(origin_card, origin, target);
    }
}