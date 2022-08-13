#ifndef __ARMEDANDDANGEROUS_CHARACTERS_H__
#define __ARMEDANDDANGEROUS_CHARACTERS_H__

#include "../card_effect.h"

namespace banggame {

    struct effect_julie_cutter : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_frankie_canton {
        opt_game_str verify(card *origin_card, player *origin, card *target);
        void on_play(card *origin_card, player *origin, card *target);
    };

    struct effect_bloody_mary : event_based_effect {
        void on_enable(card *target_card, player *target);
    };

    struct effect_red_ringo : event_based_effect {
        void on_equip(card *target_card, player *target);
    };

    struct handler_red_ringo {
        opt_game_str verify(card *origin_card, player *origin, const target_list &targets);
        void on_play(card *origin_card, player *origin, const target_list &targets);
    };

    struct effect_al_preacher : event_based_effect {
        void on_enable(card *target_card, player *target);

        bool can_respond(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct effect_ms_abigail : event_based_effect {
        bool can_escape(player *origin, card *origin_card, effect_flags flags);

        bool can_respond(card *origin_card, player *target);
        void on_play(card *origin_card, player *origin);

        void on_enable(card *origin_card, player *origin);
    };

}

#endif