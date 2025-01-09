#include "pay_gold.h"

#include "game/game_table.h"

namespace banggame {

    game_string effect_pay_gold::get_error(card_ptr origin_card, player_ptr origin) {
        if (origin->m_gold < amount) {
            return "ERROR_NOT_ENOUGH_GOLD";
        }
        return {};
    }

    void effect_pay_gold::on_play(card_ptr origin_card, player_ptr origin) {
        origin->add_gold(-amount);
    }
}