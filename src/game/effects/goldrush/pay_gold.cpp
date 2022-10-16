#include "pay_gold.h"

#include "../../game.h"

namespace banggame {

    game_string effect_pay_gold::verify(card *origin_card, player *origin) {
        if (origin->m_gold < amount) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }
        return {};
    }

    void effect_pay_gold::on_play(card *origin_card, player *origin) {
        origin->add_gold(-amount);
    }
}