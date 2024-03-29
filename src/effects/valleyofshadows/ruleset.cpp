#include "ruleset.h"

#include "effects/base/damage.h"
#include "cards/game_enums.h"

#include "game/game.h"

namespace banggame {
    
    void ruleset_valleyofshadows::on_apply(game *game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr,
            [=](card *origin_card, player *origin, const player *target, effect_flags flags, int &value) {
                if (!target->empty_hand() && bool(flags & effect_flags::escapable) && !rn::contains(game->m_discards, "ESCAPE", &card::name)) {
                    value = 1;
                }
            });

        game->add_listener<event_type::check_damage_response>(nullptr, [=](player *target, bool &value) {
            if (!value && rn::any_of(game->m_players, [target](player *p) {
                return p != target && p->alive() && !p->empty_hand();
            }) && !rn::contains(game->m_discards, "SAVED", &card::name)) {
                value = true;
            }
        });
    }
}