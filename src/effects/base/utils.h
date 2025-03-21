#ifndef __BASE_UTILS_H__
#define __BASE_UTILS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_none {};

    DEFINE_EFFECT(none, effect_none)

    struct effect_human {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(human, effect_human)

    struct effect_set_playing {
        void add_context(card_ptr origin_card, player_ptr origin, effect_context &ctx) {
            add_context(origin_card, origin, origin_card, ctx);
        }
        
        void add_context(card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx);
        
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx);
    };

    DEFINE_EFFECT(set_playing, effect_set_playing)

    struct equip_add_flag {
        player_flag flag;
        equip_add_flag(player_flag flag): flag{flag} {}
        
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(add_flag, equip_add_flag)

    struct equip_game_flag {
        game_flag flag;
        equip_game_flag(game_flag flag): flag{flag} {}
        
        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(game_flag, equip_game_flag)

}

#endif