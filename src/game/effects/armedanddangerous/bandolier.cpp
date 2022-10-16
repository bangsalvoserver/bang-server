#include "bandolier.h"

#include "../../game.h"

namespace banggame {

    game_string effect_bandolier::verify(card *origin_card, player *origin) {
        if (origin->get_bangs_played() <= 0) {
            return {"ERROR_CANT_PLAY_CARD", origin_card};
        }
        return {};
    }
}