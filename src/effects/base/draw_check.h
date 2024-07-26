#ifndef __EFFECTS_BASE_DRAW_CHECK_H__
#define __EFFECTS_BASE_DRAW_CHECK_H__

#include "cards/card_effect.h"

#include "pick.h"

namespace banggame {

    struct draw_check_handler : std::enable_shared_from_this<draw_check_handler> {
        virtual card_list get_drawn_cards() const = 0;
        virtual card *get_drawing_card() const = 0;

        virtual bool bot_check_redraw(card *target_card, player *owner) const = 0;

        virtual void resolve() = 0;
        virtual void restart() = 0;
    };
    
    using shared_request_check = std::shared_ptr<draw_check_handler>;
    
    namespace event_type {
        struct count_num_checks {
            player *origin;
            nullable_ref<int> num_checks;
        };

        struct on_draw_check_resolve {
            player *origin;
            card *target_card;
        };
        
        struct on_draw_check_select {
            player *origin;
            shared_request_check req;
            nullable_ref<bool> handled;
        };
    }
    
    struct request_check_base : selection_picker, draw_check_handler {
        request_check_base(player *target, card *origin_card)
            : selection_picker(origin_card, nullptr, target, {}, 110) {}

        card *drawn_card = nullptr;

        void on_update() override;

        game_string pick_prompt(card *target_card) const override;

        void on_pick(card *target_card) override;

        game_string status_text(player *owner) const override;

        void start();
        void select(card *target_card);

        card_list get_drawn_cards() const override {
            return {drawn_card};
        }

        card *get_drawing_card() const override {
            return origin_card;
        }

        bool is_lucky(card *target_card) const;

        bool bot_check_redraw(card *target_card, player *owner) const override;
        
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
        request_check(player *target, card *origin_card, Condition &&condition, Function &&function)
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