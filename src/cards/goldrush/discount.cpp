#include "discount.h"

#include "game/game.h"

namespace banggame {

    struct discount_obj : verify_modifier {
        discount_obj(card *origin_card, player *origin)
            : key(origin_card, -1)
            , origin(origin)
        {
            origin->m_game->add_listener<event_type::apply_cost_modifier>(key, [=](player *p, card *c, int &cost) {
                if (p == origin) {
                    --cost;
                }
            });
        }

        ~discount_obj() {
            origin->m_game->remove_listeners(key);
        }

        event_card_key key;
        player *origin;
    };

    verify_result modifier_discount::verify(card *origin_card, player *origin, card *playing_card) {
        return {std::in_place_type<discount_obj>, origin_card, origin};
    }
}