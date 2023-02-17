#include "startofturn.h"

#include "game/game.h"

namespace banggame {
    
    game_string effect_startofturn::get_error(card *origin_card, player *origin) const {
        if (origin->m_num_drawn_cards != 0) {
            return "ERROR_NOT_START_OF_TURN";
        }
        return {};
    }
}