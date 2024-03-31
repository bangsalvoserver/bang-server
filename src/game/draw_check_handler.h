#ifndef __DRAW_CHECK_HANDLER_H__
#define __DRAW_CHECK_HANDLER_H__

#include "player.h"
#include "effects/base/pick.h"

namespace banggame {

    struct draw_check_handler : std::enable_shared_from_this<draw_check_handler> {
        virtual std::vector<card *> get_drawn_cards() const = 0;
        virtual card *get_drawing_card() const = 0;

        virtual bool bot_check_redraw(card *target_card, player *owner) const = 0;

        virtual void resolve() = 0;
        virtual void restart() = 0;
    };
    
    using shared_request_check = std::shared_ptr<draw_check_handler>;
    
    namespace event_type {
        DEFINE_STRUCT(count_num_checks,
            (player *, origin)
            (nullable_ref<int>, num_checks)
        )

        DEFINE_STRUCT(on_draw_check_resolve,
            (player *, origin)
            (card *, target_card)
        )
        
        DEFINE_STRUCT(on_draw_check_select,
            (player *, origin)
            (shared_request_check, req)
            (nullable_ref<bool>, handled)
        )
    }
    
    struct request_check : selection_picker, draw_check_handler {
        request_check(game *m_game, card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target, {}, 110)
            , m_game(m_game) {}

        game *m_game;

        card *drawn_card = nullptr;

        void on_update() override;

        game_string pick_prompt(card *target_card) const override;

        void on_pick(card *target_card) override;

        game_string status_text(player *owner) const override;

        void start();
        void select(card *target_card);

        std::vector<card *> get_drawn_cards() const override {
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
    class request_check_impl : public request_check {
    private:
        Condition m_condition;
        Function m_function;

    public:
        request_check_impl(game *m_game, card *origin_card, player *target, Condition &&condition, Function &&function)
            : request_check(m_game, origin_card, target)
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