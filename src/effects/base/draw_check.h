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
            int &num_checks;
        };

        struct on_draw_check_start {
            using result_type = bool;
            player_ptr origin;
            shared_request_check req;
        };

        struct on_draw_check_resolve {
            card_ptr origin_card;
            player_ptr origin;
            card_ptr target_card;
            card_ptr drawn_card;
        };
        
        struct on_draw_check_select {
            using result_type = bool;
            player_ptr origin;
            shared_draw_check_handler req;
        };
        
        struct get_suit_modifier {
            using result_type = card_suit;
        };
    }

    struct modified_sign : card_sign {
        bool modified;
        modified_sign(card_sign sign, bool modified = false)
            : card_sign{sign}, modified{modified} {}
    };

    modified_sign get_modified_sign(const_card_ptr target_card);

    struct draw_check_result {
        bool lucky:1;
        bool rank_dependent:1;
        bool indifferent:1;
        bool defensive_redraw:1;
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

        draw_check_result get_result(card_ptr drawn_card) const;
        draw_check_result do_get_result(modified_sign sign) const;

        virtual draw_check_result get_result(card_sign sign) const = 0;
        virtual void on_resolve(card_ptr drawn_card, card_sign sign, bool lucky) = 0;
    };

    template<typename Condition, typename Function>
    class request_check : public request_check_base {
    private:
        [[no_unique_address]] Condition m_condition;
        [[no_unique_address]] Function m_function;

    public:
        request_check(player_ptr target, card_ptr origin_card, Condition &&condition, Function &&function)
            : request_check_base(target, origin_card)
            , m_condition{FWD(condition)}
            , m_function{FWD(function)} {}
        
        draw_check_result get_result(card_sign sign) const override {
            if constexpr (invocable_like<Condition, draw_check_result(card_sign)>) {
                return std::invoke(m_condition, sign);
            } else if constexpr (std::predicate<Condition, card_sign>) {
                return { .lucky = std::invoke(m_condition, sign) };
            } else {
                static_assert(false, "Invalid request_check Condition type");
            }
        }

        void on_resolve(card_ptr drawn_card, card_sign sign, bool lucky) override {
            if constexpr (std::invocable<Function, card_ptr, card_sign, bool>) {
                std::invoke(m_function, drawn_card, sign, lucky);
            } else if constexpr (std::invocable<Function, card_sign, bool>) {
                std::invoke(m_function, sign, lucky);
            } else if constexpr (std::invocable<Function, card_sign>) {
                std::invoke(m_function, sign);
            } else if constexpr (std::invocable<Function, bool>) {
                std::invoke(m_function, lucky);
            } else {
                static_assert(false, "Invalid request_check Function type");
            }
        }
    };

}

#endif