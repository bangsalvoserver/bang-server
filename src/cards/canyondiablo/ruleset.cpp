#include "ruleset.h"

#include "game/game.h"

namespace banggame {
    void ruleset_canyondiablo::on_apply(game *game) {
        game->add_listener<event_type::check_damage_response>(nullptr, [=](player *target, bool &value) {
            if (!value && std::ranges::any_of(range_other_players(target), std::not_fn(&player::empty_hand))
                && !ranges_contains(game->m_discards, "SACRIFICE", &card::name))
            {
                value = true;
            }
        });
    }
}