#include "max_usages.h"

#include "game/game.h"

namespace banggame {
    game_string effect_max_usages::verify(card *origin_card, player *origin) {
        if (origin->m_game->call_event<event_type::count_usages>(origin, origin_card, 0) >= max_usages) {
            return {"ERROR_MAX_USAGES", origin_card, max_usages};
        }
        return {};
    }
    
    void effect_max_usages::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 5};
        origin->m_game->add_listener<event_type::count_usages>(key, [=](player *e_origin, card *e_card, int &usages) {
            if (origin_card == e_card && origin == e_origin) {
                ++usages;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *e_origin, bool skipped) {
            if (e_origin == origin) {
                origin->m_game->remove_listeners(key);
            }
        });
    }
}