#include "ruleset.h"

#include "game/game_table.h"
#include "game/game_options.h"

#include "effects/base/damage.h"
#include "effects/dodgecity/ruleset.h"

namespace banggame {
    
    using namespace std::string_view_literals;
    
    void ruleset_canyondiablo::on_apply(game_ptr game) {
        game->add_listener<event_type::check_damage_response>(nullptr, [=](player_ptr target, bool &value) {
            if (!value && rn::any_of(game->m_players, [target](player_ptr p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !rn::contains(game->m_discards, "SACRIFICE"sv, &card::name)) {
                value = true;
            }
        });

        if (!game->m_options.expansions.contains(GET_RULESET(dodgecity))) {
            ruleset_dodgecity{}.on_apply(game);
        }
    }
}