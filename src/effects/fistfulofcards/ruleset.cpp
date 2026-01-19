#include "ruleset.h"

#include "game/game_table.h"
#include "game/game_options.h"

#include "cards/game_events.h"

#include "effects/highnoon/ruleset.h"
#include "effects/base/death.h"

namespace banggame {

    void ruleset_fistfulofcards::on_apply(game_ptr game) {
        if (!game->m_options.expansions.contains(GET_RULESET(highnoon))) {
            ruleset_highnoon{}.on_apply(game);
        }

        game->add_listener<event_type::on_player_eliminated>(nullptr, [first = true](player_ptr origin, player_ptr target, death_type type) mutable {
            if (type == death_type::death && std::exchange(first, false)) {
                target->m_game->add_listener<event_type::get_first_dead_player>(nullptr, [=]{ return target; });
            }
        });
    }
}