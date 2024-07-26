#include "add_gold.h"

#include "game/game.h"

namespace banggame {

    void effect_add_gold::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->add_gold(amount);
    }
}