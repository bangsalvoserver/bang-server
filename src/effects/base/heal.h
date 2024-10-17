#ifndef __EFFECT_HEAL_H__
#define __EFFECT_HEAL_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_heal {
        int amount;
        effect_heal(int value) : amount(std::max(1, value)) {}

        game_string on_prompt(card_ptr origin_card, player_ptr origin) {
            return on_prompt(origin_card, origin, origin);
        }
        game_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);

        void on_play(card_ptr origin_card, player_ptr origin, effect_flags flags = {}) {
            on_play(origin_card, origin, origin, flags);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {});
    };

    DEFINE_EFFECT(heal, effect_heal)

    struct effect_heal_notfull : effect_heal {
        using effect_heal::effect_heal;

        game_string get_error(card_ptr origin_card, player_ptr origin) {
            return get_error(origin_card, origin, origin);
        }
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(heal_notfull, effect_heal_notfull)
}

#endif