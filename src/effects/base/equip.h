#ifndef __BASE_EQUIP_H__
#define __BASE_EQUIP_H__

#include "cards/card_effect.h"

namespace banggame {

    const player_filter_bitset *get_equip_filter(const_card_ptr origin_card);

    game_string get_equip_error(const_card_ptr origin_card, player_ptr target);

    struct effect_equip_on {
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx = {});
        void add_context(card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx);
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx);
    };

    DEFINE_EFFECT(equip_on, effect_equip_on)

    namespace event_type {
        struct check_equip_card {
            using result_type = game_string;
            player_ptr origin;
            card_ptr origin_card;
            const_player_ptr target;
            const effect_context &ctx;
        };
    }

}

#endif