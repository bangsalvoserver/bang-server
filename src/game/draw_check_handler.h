#ifndef __DRAW_CHECK_HANDLER_H__
#define __DRAW_CHECK_HANDLER_H__

#include "player.h"

namespace banggame {

    struct draw_check_handler {
        virtual std::vector<card *> get_drawn_cards() const = 0;
        virtual card *get_drawing_card() const = 0;

        virtual bool bot_check_redraw(card *target_card, player *owner) const = 0;

        virtual void resolve() = 0;
        virtual void restart() = 0;
    };
    
    struct request_check : selection_picker, draw_check_handler {
        request_check(game *m_game, card *origin_card, player *target, draw_check_condition &&condition, draw_check_function &&function)
            : selection_picker(origin_card, nullptr, target)
            , m_game(m_game)
            , m_condition(std::move(condition))
            , m_function(std::move(function)) {}

        game *m_game;
        draw_check_condition m_condition;
        draw_check_function m_function;

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
    };

}

#endif