#include "miss_susanna.h"

#include "game/game.h"

namespace banggame {

    void equip_miss_susanna::on_enable(card *target_card, player *target) {
        // TODO

        // add listener on_effect_end(2)
        //      add_listener count_played_cards (+1)

        // add listener on_turn_end(3)
        //      if call_event count_played_cards < 3 -> damage(1)
        //      remove_listeners (2)
    }
}