#ifndef __DRAW_CHECK_HANDLER_H__
#define __DRAW_CHECK_HANDLER_H__

#include "player.h"

namespace banggame {

    using draw_check_condition = std::function<bool(card_sign)>;
    using draw_check_function = std::function<void(bool)>;

    class draw_check_handler {
    private:
        player *m_origin = nullptr;
        card *m_origin_card = nullptr;
        draw_check_condition m_condition;
        draw_check_function m_function;

    public:
        void set(player *origin, card *origin_card, draw_check_condition &&condition, draw_check_function &&function);
        void start();
        void select(card *drawn_card);
        void restart();
        bool check(card *drawn_card);
        void resolve(card *drawn_card);
    };

}

#endif