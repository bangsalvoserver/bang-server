#include "ruleset.h"

#include "game/game.h"

namespace banggame {
    void ruleset_canyondiablo::on_apply(game *game) {
        game->add_listener<event_type::check_damage_response>(nullptr, [=](bool &value) {
            value = value || !ranges_contains(game->m_discards, "SACRIFICE", &card::name);
        });
    }
}