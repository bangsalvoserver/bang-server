#ifndef __EFFECTS_BASE_DRAW_CHECK_H__
#define __EFFECTS_BASE_DRAW_CHECK_H__

#include "cards/card_effect.h"

#include "pick.h"

namespace banggame {

    struct draw_check_handler : std::enable_shared_from_this<draw_check_handler> {
        virtual card_list get_drawn_cards() const = 0;
        virtual card_ptr get_drawing_card() const = 0;

        virtual game_string redraw_prompt(card_ptr target_card, player_ptr owner) const = 0;

        virtual void resolve() = 0;
        virtual void restart() = 0;
    };
    
    using shared_request_check = std::shared_ptr<draw_check_handler>;
    
    namespace event_type {
        struct count_num_checks {
            const_player_ptr origin;
            nullable_ref<int> num_checks;
        };

        struct on_draw_check_resolve {
            player_ptr origin;
            card_ptr target_card;
        };
        
        struct on_draw_check_select {
            player_ptr origin;
            shared_request_check req;
            nullable_ref<bool> handled;
        };
    }
    
    struct request_check_base : selection_picker, draw_check_handler {
        request_check_base(player_ptr target, card_ptr origin_card)
            : selection_picker(origin_card, nullptr, target, {}, 110) {}

        card_ptr drawn_card = nullptr;

        void on_update() override;

        game_string pick_prompt(card_ptr target_card) const override;

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

        bool is_lucky(card_ptr target_card) const;

        game_string redraw_prompt(card_ptr target_card, player_ptr owner) const override;
        
        void resolve() override;
        void restart() override;

        virtual bool check_condition(card_sign sign) const = 0;
        virtual void on_resolve(bool result) = 0;
    };

    template<typename T>
    concept draw_check_condition = std::predicate<T, card_sign>;

    template<typename T>
    concept draw_check_function = std::invocable<T, bool>;

    template<draw_check_condition Condition, draw_check_function Function>
    class request_check : public request_check_base {
    private:
        Condition m_condition;
        Function m_function;

    public:
        request_check(player_ptr target, card_ptr origin_card, Condition &&condition, Function &&function)
            : request_check_base(target, origin_card)
            , m_condition(FWD(condition))
            , m_function(FWD(function)) {}
        
        bool check_condition(card_sign sign) const override {
            return std::invoke(m_condition, sign);
        }

        void on_resolve(bool result) override {
            std::invoke(m_function, result);
        }
    };

}

#endif