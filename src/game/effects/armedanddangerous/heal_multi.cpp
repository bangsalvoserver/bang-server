#include "heal_multi.h"

#include "../../game.h"
#include "../base/effects.h"

namespace banggame {

    game_string handler_heal_multi::on_prompt(card *origin_card, player *origin, int amount) {
        return effect_heal(amount).on_prompt(origin_card, origin);
    }

    void handler_heal_multi::on_play(card *origin_card, player *origin, int amount) {
        effect_heal(amount).on_play(origin_card, origin);
    }
}