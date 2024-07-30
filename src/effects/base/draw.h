#ifndef __BASE_DRAW_H__
#define __BASE_DRAW_H__

#include "cards/card_effect.h"
#include "pick.h"

namespace banggame {
    
    struct effect_draw {
        int ncards;
        effect_draw(int value) : ncards(std::max(1, value)) {}
        
        void on_play(card_ptr origin_card, player_ptr origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(draw, effect_draw)

    struct effect_queue_draw {
        int ncards;
        effect_queue_draw(int value) : ncards(std::max(1, value)) {}

        void on_play(card_ptr origin_card, player_ptr origin) {
            on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(queue_draw, effect_queue_draw)

    struct effect_draw_discard {
        game_string get_error(card_ptr origin_card, player_ptr origin) {
            return get_error(origin_card, origin, origin);
        }
        game_string get_error(card_ptr origin_card, player_ptr origin, player_ptr target);

        void on_play(card_ptr origin_card, player_ptr origin) {
            return on_play(origin_card, origin, origin);
        }
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target);
    };

    DEFINE_EFFECT(draw_discard, effect_draw_discard)

    struct effect_draw_to_discard {
        int ncards;
        effect_draw_to_discard(int value) : ncards(std::max(1, value)) {}

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

    struct interface_drawing : std::enable_shared_from_this<interface_drawing> {
        virtual int get_drawn_cards() const = 0;
        virtual int get_cards_to_draw() const = 0;
        virtual void add_drawn_cards(int ncards) = 0;
        virtual void add_cards_to_draw(int ncards) = 0;
        
        virtual card_ptr phase_one_drawn_card() = 0;
        virtual void add_to_hand_phase_one(card *target_card) = 0;
    };
    
    using shared_request_draw = std::shared_ptr<interface_drawing>;

    class request_draw : public request_picking, public interface_drawing {
    private:
        int num_drawn_cards = 0;
        int num_cards_to_draw = 2;

    public:
        request_draw(player_ptr target);

        int get_drawn_cards() const override {
            return num_drawn_cards;
        }

        int get_cards_to_draw() const override {
            return num_cards_to_draw;
        }

        void add_drawn_cards(int ncards) override {
            num_drawn_cards += ncards;
        }

        void add_cards_to_draw(int ncards) override {
            num_cards_to_draw += ncards;
        }
        
        card_ptr phase_one_drawn_card() override;
        void add_to_hand_phase_one(card_ptr target_card) override;

        void on_update() override;
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };

    namespace event_type {
        struct count_cards_to_draw {
            player_ptr origin;
            nullable_ref<int> cards_to_draw;
        };

        struct pre_draw_from_deck {
            player_ptr origin;
            shared_request_draw req;
            nullable_ref<bool> handled;
        };
        
        struct on_draw_from_deck {
            player_ptr origin;
            shared_request_draw req;
            nullable_ref<bool> handled;
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