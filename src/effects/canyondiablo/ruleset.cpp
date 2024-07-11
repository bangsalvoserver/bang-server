#include "ruleset.h"

#include "game/game.h"

#include "effects/base/damage.h"
#include "effects/dodgecity/ruleset.h"

namespace banggame {
    void ruleset_canyondiablo::on_apply(game *game) {
        game->add_listener<event_type::check_damage_response>(nullptr, [=](player *target, bool &value) {
            if (!value && rn::any_of(game->m_players, [target](player *p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !rn::contains(game->m_discards, "SACRIFICE", &card::name)) {
                value = true;
            }
        });

        if (!bool(game->m_options.expansions & expansion_type::dodgecity)) {
            ruleset_dodgecity{}.on_apply(game);
        }
    }
}