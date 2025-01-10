#include "escapable.h"

#include "game/game_table.h"

namespace banggame {

    escape_type get_escape_type(player_ptr origin, player_ptr target, card_ptr origin_card, effect_flags flags) {
        escape_type result = escape_type::no_escape;
        target->m_game->call_event(event_type::apply_escapable_modifier{ origin_card, origin, target, flags, result });
        return result;
    }

}