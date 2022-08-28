#ifndef __BASE_EFFECTS_H__
#define __BASE_EFFECTS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_play_card_action {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_max_usages {
        int max_usages;
        
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_pass_turn {
        game_string verify(card *origin_card, player *origin);
        game_string on_prompt(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_resolve {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };
    
    struct effect_bang {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    struct handler_bangcard {
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct handler_play_as_bang {
        void on_play(card *origin_card, player *origin, card *chosen_card, player *target);
    };

    struct handler_play_as_missed {
        void on_play(card *origin_card, player *origin, card *chosen_card);
    };

    struct effect_banglimit {
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_indians {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    struct effect_duel {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    struct effect_missedlike {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin) {}
    };
    
    struct effect_missed : effect_missedlike {
        void on_play(card *origin_card, player *origin);
    };
    
    struct effect_barrel : effect_missed {
        void on_play(card *origin_card, player *target);
    };

    struct effect_beer {
        game_string on_prompt(card *origin_card, player *origin) {
            return on_prompt(origin_card, origin, origin);
        }
        game_string on_prompt(card *origin_card, player *origin, player *target);
        
        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct effect_heal {
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
        game_string verify(card *origin_card, player *origin) {
            return verify(origin_card, origin, origin);
        }
        game_string verify(card *origin_card, player *origin, player *target);
    };

    struct effect_deathsave {
        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct prompt_target_self_hand {
        game_string on_prompt(card *origin_card, player *origin, card *target);
    };

    struct effect_steal : prompt_target_self_hand {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});

        struct resolver {
            card *origin_card;
            player *origin;
            card *target_card;

            void resolve() const;
            void finalize() const;
        };
    };

    struct effect_discard {
        void on_play(card *origin_card, player *origin, card *target, effect_flags flags = {});
        
        struct resolver {
            card *origin_card;
            player *origin;
            card *target_card;

            void resolve() const;
            void finalize() const;
        };
    };

    struct effect_destroy : effect_discard, prompt_target_self_hand {};

    struct effect_choose_card {
        void on_play(card *origin_card, player *origin, card *target);
    };

    struct effect_draw {
        int ncards;
        effect_draw(int value) : ncards(std::max(1, value)) {}
        
        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct effect_draw_discard {
        game_string verify(card *origin_card, player *origin) {
            return verify(origin_card, origin, origin);
        }
        game_string verify(card *origin_card, player *origin, player *target);

        void on_play(card *origin_card, player *origin) {
            return on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct effect_draw_to_discard {
        int ncards;
        effect_draw_to_discard(int value) : ncards(std::max(1, value)) {}

        void on_play(card *origin_card, player *origin);
    };

    struct effect_while_drawing : effect_empty {
        bool can_respond(card *origin_card, player *origin);
    };

    struct effect_end_drawing : effect_while_drawing {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_draw_one_less {
        void on_play(card *origin_card, player *target);
    };

    struct effect_generalstore {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_self_damage {
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

}

#endif