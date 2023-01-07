#include "add_cost.h"

#include "game/game.h"

namespace banggame {

    verify_result effect_add_cost::verify(card *origin_card, player *origin) {
        struct modifier_cost : verify_modifier {
            modifier_cost(card *origin_card, player *origin, int value)
                : key(origin_card, -1)
                , origin(origin)
            {
                origin->m_game->add_listener<event_type::apply_cost_modifier>(key, [origin=origin, value](player *p, card *c, int &cost) {
                    if (p == origin) {
                        cost += value;
                    }
                });
            }

            ~modifier_cost() {
                origin->m_game->remove_listeners(key);
            }

            event_card_key key;
            player *origin;
        };

        return {std::in_place_type<modifier_cost>, origin_card, origin, value};
    }
}