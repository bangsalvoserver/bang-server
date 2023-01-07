#include "belltower.h"

#include "game/game.h"

namespace banggame {

    verify_result effect_belltower::verify(card *origin_card, player *origin) {
        struct modifier_belltower : verify_modifier {
            modifier_belltower(card *origin_card, player *origin)
                : key(origin_card, -1)
                , origin(origin)
            {
                origin->m_game->add_listener<event_type::apply_distance_modifier>(key, [origin=origin](player *p, int &value) {
                    if (p == origin) {
                        value = 1;
                    }
                });
            }

            ~modifier_belltower() {
                origin->m_game->remove_listeners(key);
            }

            event_card_key key;
            player *origin;
        };

        return {std::in_place_type<modifier_belltower>, origin_card, origin};
    }
}