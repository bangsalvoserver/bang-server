#include "ruleset.h"

#include "game/game.h"

namespace banggame {
    
    void ruleset_valleyofshadows::on_apply(game *game) {
        game->add_listener<event_type::apply_escapable_modifier>(nullptr, [=](card *origin_card, player *origin, const player *target, effect_flags flags, bool &value) {
            value = value || bool(flags & effect_flags::escapable) && !ranges_contains(game->m_discards, "ESCAPE", &card::name);
        });

        game->add_listener<event_type::check_damage_response>(nullptr, [=](player *target, bool &value) {
            if (!value && std::ranges::any_of(range_other_players(target), std::not_fn(&player::empty_hand))
                && !ranges_contains(game->m_discards, "SAVED", &card::name))
            {
                value = true;
            }
        });
    }
}