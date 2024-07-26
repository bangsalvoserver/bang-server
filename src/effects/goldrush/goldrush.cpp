#include "goldrush.h"

#include "game/game.h"

namespace banggame {
    void effect_goldrush::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->add_listener<event_type::on_turn_end>(origin_card, [=](player_ptr p, bool skipped) {
            if (p == origin) {
                origin->heal(origin->m_max_hp);
                origin->m_game->remove_listeners(origin_card);
            }
        });
        ++origin->m_extra_turns;
    }
}