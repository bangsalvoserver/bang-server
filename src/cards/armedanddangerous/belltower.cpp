#include "belltower.h"

#include "game/game.h"

namespace banggame {
    
    struct belltower_obj : verify_modifier {
        belltower_obj(card *origin_card, player *origin)
            : key(origin_card, -1)
            , origin(origin)
        {
            origin->m_game->add_listener<event_type::apply_distance_modifier>(key, [=](player *p, int &value) {
                if (p == origin) {
                    value = 1;
                }
            });
        }

        ~belltower_obj() {
            origin->m_game->remove_listeners(key);
        }

        event_card_key key;
        player *origin;
    };

    verify_result modifier_belltower::verify(card *origin_card, player *origin, card *playing_card) {
        return {std::in_place_type<belltower_obj>, origin_card, origin};
    }
}