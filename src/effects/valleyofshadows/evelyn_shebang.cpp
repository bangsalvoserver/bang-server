#include "evelyn_shebang.h"

#include "cards/game_events.h"

#include "effects/base/bang.h"

#include "game/game_table.h"
#include "game/prompts.h"

namespace banggame {

    game_string effect_evelyn_shebang::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        return origin->m_game->call_event(event_type::check_bang_target{ origin_card, origin, target, effect_flags{} });
    }

    prompt_string effect_evelyn_shebang::on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target) {
        return effect_bang{}.on_prompt(origin_card, origin, target);
    }

    void effect_evelyn_shebang::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->add_listener<event_type::check_bang_target>(origin_card, [=](card_ptr e_origin_card, player_ptr e_origin, player_ptr e_target, effect_flags flags) -> game_string {
            if (e_origin_card == origin_card && e_origin == origin && e_target == target) {
                return "ERROR_TARGET_NOT_UNIQUE";
            }
            return {};
        });

        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player_ptr e_origin, bool skipped) {
            if (origin == e_origin) {
                origin->m_game->remove_listeners(origin_card);
            }
        });

        effect_bang{}.on_play(origin_card, origin, target);
    }
}