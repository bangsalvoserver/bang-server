#ifndef __ARMEDANDDANGEROUS_EFFECTS_H__
#define __ARMEDANDDANGEROUS_EFFECTS_H__

#include "../card_effect.h"

namespace banggame {

    struct handler_draw_atend {
        void on_play(card *origin_card, player *origin, size_t amount);
    };

    struct handler_heal_multi {
        opt_game_str on_prompt(card *origin_card, player *origin, size_t amount);
        void on_play(card *origin_card, player *origin, size_t amount);
    };

    struct effect_select_cube {
        opt_game_str verify(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };

    struct effect_pay_cube {
        int ncubes;
        effect_pay_cube(int value) : ncubes(std::max(1, value)) {}
        
        opt_game_str verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_add_cube {
        int ncubes;
        effect_add_cube(int value) : ncubes(std::max(1, value)) {}

        void on_play(card *origin_card, player *origin, card *target);
    };
    
    struct effect_reload {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_rust {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
        void on_resolve(card *origin_card, player *origin, player *target);
    };

    struct effect_doublebarrel {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_thunderer {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_buntlinespecial {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_bigfifty {
        void on_play(card *origin_card, player *origin);
    };

    struct handler_flintlock {
        void on_play(card *origin_card, player *origin, player *target, opt_tagged_value<target_type::none> paid_cubes);
    };

    struct effect_bandolier : effect_empty {
        opt_game_str verify(card *origin_card, player *origin);
    };

    struct handler_duck {
        void on_play(card *origin_card, player *origin, opt_tagged_value<target_type::none> paid_cubes);
    };

    struct handler_squaw {
        opt_game_str verify(card *origin_card, player *origin, card *discarded_card, opt_tagged_value<target_type::cube> paid_cubes);
        void on_play(card *origin_card, player *origin, card *discarded_card, opt_tagged_value<target_type::cube> paid_cubes);
    };

    struct effect_move_bomb : effect_empty {
        bool can_respond(card *origin_card, player *origin);
    };

    struct handler_move_bomb {
        opt_game_str on_prompt(card *origin_card, player *origin, player *target);
        opt_game_str verify(card *origin_card, player *origin, player *target);
        void on_play(card *origin_card, player *origin, player *target);
    };
}

#endif