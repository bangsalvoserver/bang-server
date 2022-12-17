#include "ruleset.h"

#include "game/game.h"

namespace banggame {
    void ruleset_goldrush::on_apply(game *game) {
        game->add_listener<event_type::on_hit>({nullptr, 5}, [=](card *origin_card, player *origin, player *target, int damage, effect_flags flags) {
            if (origin && game->m_playing == origin && origin != target && origin->alive()) {
                origin->add_gold(damage);
            }
        });
    }
}