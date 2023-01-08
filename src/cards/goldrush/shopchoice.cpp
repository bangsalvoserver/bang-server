#include "shopchoice.h"

#include "game/game.h"

namespace banggame {

    struct shopchoice_obj : verify_modifier {
        shopchoice_obj(card *origin_card, player *origin)
            : key(origin_card, -1)
            , origin(origin)
        {
            origin->m_game->add_listener<event_type::apply_cost_modifier>(key, [=](player *p, card *c, int &cost) {
                if (p == origin) {
                    cost += origin_card->buy_cost();
                }
            });
        }

        ~shopchoice_obj() {
            origin->m_game->remove_listeners(key);
        }

        event_card_key key;
        player *origin;
    };

    verify_result modifier_shopchoice::verify(card *origin_card, player *origin, card *playing_card) {
        return {std::in_place_type<shopchoice_obj>, origin_card, origin};
    }
}