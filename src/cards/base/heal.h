#ifndef __EFFECT_HEAL_H__
#define __EFFECT_HEAL_H__

#include "cards/card_effect.h"
#include "game/bot_suggestion.h"

namespace banggame {

    struct effect_heal : bot_suggestion::target_friend {
        int amount;
        effect_heal(int value) : amount(std::max(1, value)) {}

        game_string on_prompt(card *origin_card, player *origin) {
            return on_prompt(origin_card, origin, origin);
        }
        game_string on_prompt(card *origin_card, player *origin, player *target);

        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct effect_heal_notfull : effect_heal {
        using effect_heal::effect_heal;

        game_string get_error(card *origin_card, player *origin) {
            return get_error(origin_card, origin, origin);
        }
        game_string get_error(card *origin_card, player *origin, player *target);
    };

    struct effect_queue_heal_notfull : effect_heal_notfull {
        using effect_heal_notfull::effect_heal_notfull;
        
        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif