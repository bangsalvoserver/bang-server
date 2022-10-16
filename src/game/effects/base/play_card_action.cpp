#include "play_card_action.h"

#include "../../game.h"

namespace banggame {
    
    void effect_play_card_action::on_play(card *origin_card, player *origin) {
        origin->play_card_action(origin_card);
    }
    
    void event_based_effect::on_disable(card *target_card, player *target) {
        target->m_game->remove_listeners(target_card);
    }
}