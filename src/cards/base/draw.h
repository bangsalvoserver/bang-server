#ifndef __BASE_DRAW_H__
#define __BASE_DRAW_H__

#include "cards/card_effect.h"

namespace banggame {
    
    struct effect_draw {
        int ncards;
        effect_draw(int value) : ncards(std::max(1, value)) {}
        
        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };
    
    struct handler_draw_multi {
        void on_play(card *origin_card, player *origin, int amount);
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

    struct effect_while_drawing {
        bool can_respond(card *origin_card, player *origin);
    };

    struct effect_reset_drawing : effect_while_drawing {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_end_drawing : effect_while_drawing {
        void on_play(card *origin_card, player *origin);
    };

    struct effect_draw_one_less {
        void on_play(card *origin_card, player *target);
    };

    struct request_draw : request_base {
        request_draw(player *target)
            : request_base(nullptr, nullptr, target, effect_flags::auto_pick) {}

        void on_update() override;
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };
}

#endif