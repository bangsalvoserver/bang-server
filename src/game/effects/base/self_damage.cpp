#include "self_damage.h"

#include "../../game.h"

namespace banggame {
    
    game_string effect_self_damage::verify(card *origin_card, player *origin) {
        if (origin->m_hp <= 1) {
            return "ERROR_CANT_SELF_DAMAGE";
        }
        return {};
    }

    void effect_self_damage::on_play(card *origin_card, player *origin) {
        origin->damage(origin_card, origin, 1);
    }
}