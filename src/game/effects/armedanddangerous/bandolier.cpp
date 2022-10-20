#include "bandolier.h"

#include "../../game.h"

namespace banggame {

    void effect_bandolier::on_play(card *origin_card, player *origin) {
        event_card_key key{origin_card, 4};
        origin->m_game->add_listener<event_type::count_bangs_played>(key, [=](player *p, int &value) {
            if (origin == p) {
                --value;
            }
        });
        origin->m_game->add_listener<event_type::on_turn_end>(key, [=](player *p, bool skipped) {
            if (origin == p) {
                origin->m_game->remove_listeners(key);
            }
        });
    }
}