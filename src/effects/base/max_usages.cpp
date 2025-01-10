#include "max_usages.h"

#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {
    game_string effect_max_usages::get_error(card_ptr origin_card, player_ptr origin) {
        int usages = 0;
        origin->m_game->call_event(event_type::count_usages{ origin, origin_card, usages });
        if (usages >= max_usages) {
            return {"ERROR_MAX_USAGES", origin_card, max_usages};
        }
        return {};
    }
    
    void effect_max_usages::on_play(card_ptr origin_card, player_ptr origin) {
        event_card_key key{origin_card, 5};
        origin->m_game->add_listener<event_type::count_usages>(key, [=](player_ptr e_origin, card_ptr e_card, int &usages) {
            if (origin_card == e_card && origin == e_origin) {
                ++usages;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player_ptr e_origin, bool skipped) {
            origin->m_game->remove_listeners(key);
        });
    }
}