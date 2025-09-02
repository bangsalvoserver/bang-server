#ifndef __BASE_DRAW_H__
#define __BASE_DRAW_H__

#include "cards/card_effect.h"
#include "pick.h"

namespace banggame {
    
    struct effect_draw {
        int ncards;
        effect_draw(int ncards = 1) : ncards{ncards} {}
        
        void on_play(card_ptr origin_card, player_ptr origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(draw, effect_draw)

    struct effect_queue_draw {
        int ncards;
        effect_queue_draw(int ncards = 1) : ncards{ncards} {}

        void on_play(card_ptr origin_card, player_ptr origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(queue_draw, effect_queue_draw)

    struct effect_draw_discard {
        bool can_play(card_ptr origin_card, player_ptr origin);

        void on_play(card_ptr origin_card, player_ptr origin) {
            return on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(draw_discard, effect_draw_discard)

    struct effect_draw_to_discard {
        int ncards;
        effect_draw_to_discard(int ncards = 1) : ncards{ncards} {}

        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(draw_to_discard, effect_draw_to_discard)

    struct effect_while_drawing {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(while_drawing, effect_while_drawing)

    struct effect_no_cards_drawn {
        bool can_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(no_cards_drawn, effect_no_cards_drawn)

    struct effect_add_draw_card: effect_while_drawing {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(add_draw_card, effect_add_draw_card)

    struct effect_skip_drawing: effect_while_drawing {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(skip_drawing, effect_skip_drawing)

    struct request_draw : request_picking, std::enable_shared_from_this<request_draw> {
        request_draw(player_ptr target);

        card_list handlers;

        card_list cards_from_selection;
        
        int num_drawn_cards = 0;
        int num_cards_to_draw = 2;
        
        card_ptr phase_one_drawn_card();
        void add_to_hand_phase_one(card_ptr target_card);
        void cleanup_selection();

        void on_update() override;
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };
    
    using shared_request_draw = std::shared_ptr<request_draw>;

    namespace event_type {
        struct count_cards_to_draw {
            player_ptr origin;
            nullable_ref<int> cards_to_draw;
        };

        struct get_draw_handlers {
            player_ptr origin;
            shared_request_draw req;
        };
        
        struct on_draw_from_deck {
            player_ptr origin;
            card_ptr target_card;
            shared_request_draw req;
        };

        struct on_card_drawn {
            player_ptr origin;
            card_ptr target_card;
            shared_request_draw req;
            nullable_ref<bool> reveal;
        };
    }
}

#endif