#ifndef __EFFECTS_BASE_DRAW_CHECK_H__
#define __EFFECTS_BASE_DRAW_CHECK_H__

#include "cards/card_effect.h"

#include "pick.h"

namespace banggame {

    struct draw_check_handler {
        virtual card_list get_drawn_cards() const = 0;
        virtual card_ptr get_drawing_card() const = 0;

        virtual prompt_string redraw_prompt(card_ptr target_card, player_ptr owner) const = 0;

        virtual void resolve() = 0;
        virtual void restart() = 0;
    };

    struct request_check_base;
    
    using shared_request_check = std::shared_ptr<request_check_base>;
    using shared_draw_check_handler = std::shared_ptr<draw_check_handler>;
    
    namespace event_type {
        struct count_num_checks {
            const_player_ptr origin;
            nullable_ref<int> num_checks;
        };

        struct on_draw_check_start {
            player_ptr origin;
            shared_request_check req;
            nullable_ref<bool> handled;
        };

        struct on_draw_check_resolve {
            card_ptr origin_card;
            player_ptr origin;
            card_ptr target_card;
            card_ptr drawn_card;
        };
        
        struct on_draw_check_select {
            player_ptr origin;
            shared_draw_check_handler req;
            nullable_ref<bool> handled;
        };
        
        struct apply_sign_modifier {
            nullable_ref<card_sign> value;
        };
    }
    
    card_sign get_modified_sign(const_card_ptr target_card);

    struct draw_check_result {
        bool lucky;
        bool indifferent;
    };
    
    struct request_check_base : selection_picker, draw_check_handler, std::enable_shared_from_this<request_check_base> {
        request_check_base(player_ptr target, card_ptr origin_card)
            : selection_picker(origin_card, nullptr, target, {}, 110) {}

        card_ptr drawn_card = nullptr;

        void on_update() override;

        prompt_string pick_prompt(card_ptr target_card) const override;

        void on_pick(card_ptr target_card) override;

        game_string status_text(player_ptr owner) const override;

        void start();
        void select(card_ptr target_card);

        card_list get_drawn_cards() const override {
            return {drawn_card};
        }

        card_ptr get_drawing_card() const override {
            return origin_card;
        }

        prompt_string redraw_prompt(card_ptr target_card, player_ptr owner) const override;
        
        void resolve() override;
        void restart() override;

        virtual draw_check_result get_result(card_ptr target_card) const = 0;
        virtual void on_resolve(draw_check_result result) = 0;
    };

    template<typename T> struct draw_check_condition_wrapper;

    template<invocable_like<draw_check_result(card_ptr)> T>
    struct draw_check_condition_wrapper<T> {
        [[no_unique_address]] T fun;

        draw_check_result operator()(card_ptr target_card) const {
            return std::invoke(fun, target_card);
        }
    };

    template<std::predicate<card_sign> T>
    struct draw_check_condition_wrapper<T> {
        [[no_unique_address]] T fun;

        draw_check_result operator()(card_ptr target_card) const {
            return draw_check_result{ .lucky = std::invoke(fun, get_modified_sign(target_card)) };
        }
    };

    template<typename T>
    concept draw_check_condition = requires {
        sizeof(draw_check_condition_wrapper<T>);
    };

    template<typename T>
    concept draw_check_function = std::invocable<T, bool>;

    template<draw_check_condition Condition, draw_check_function Function>
    class request_check : public request_check_base {
    private:
        [[no_unique_address]] draw_check_condition_wrapper<Condition> m_condition;
        [[no_unique_address]] Function m_function;

    public:
        request_check(player_ptr target, card_ptr origin_card, Condition &&condition, Function &&function)
            : request_check_base(target, origin_card)
            , m_condition{FWD(condition)}
            , m_function{FWD(function)} {}
        
        draw_check_result get_result(card_ptr target_card) const override {
            return std::invoke(m_condition, target_card);
        }

        void on_resolve(draw_check_result result) override {
            std::invoke(m_function, result.lucky);
        }
    };

}

#endif