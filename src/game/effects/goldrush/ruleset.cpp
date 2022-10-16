#include "ruleset.h"

#include "../../game.h"

namespace banggame {
    void ruleset_goldrush::on_apply(game *game) {
        game->add_listener<event_type::before_hit>({nullptr, 1}, [=](card *origin_card, player *origin, player *target, int damage, bool is_bang) {
            if (origin && game->m_playing == origin && origin != target && origin->alive()) {
                origin->add_gold(damage);
            }
        });
    }
}