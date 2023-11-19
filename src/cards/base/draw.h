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

    struct effect_queue_draw {
        int ncards;
        effect_queue_draw(int value) : ncards(std::max(1, value)) {}

        void on_play(card *origin_card, player *origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card *origin_card, player *origin, player *target);
    };
    
    struct handler_draw_multi {
        void on_play(card *origin_card, player *origin, int amount);
    };

    struct effect_draw_discard {
        game_string get_error(card *origin_card, player *origin) {
            return get_error(origin_card, origin, origin);
        }
        game_string get_error(card *origin_card, player *origin, player *target);

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

    struct effect_startofturn {
        game_string get_error(card *origin_card, player *origin) const;
    };

    struct effect_while_drawing {
        int cards_to_add;
        effect_while_drawing(int value): cards_to_add(value) {}
        
        bool can_play(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    struct request_draw : request_base, std::enable_shared_from_this<request_draw> {
        request_draw(player *target)
            : request_base(nullptr, nullptr, target, {}, -7) {}

        int num_drawn_cards = 0;
        
        card *phase_one_drawn_card();
        void add_to_hand_phase_one(card *target_card);

        void on_update() override;
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };
}

#endif