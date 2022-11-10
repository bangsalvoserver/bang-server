#include "play_card_action.h"

#include "game/game.h"

namespace banggame {
    
    void effect_play_card_action::on_play(card *origin_card, player *origin) {
        origin->play_card_action(origin_card);
    }
}