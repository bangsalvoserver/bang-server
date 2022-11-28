#ifndef __BASE_PREDRAW_CHECK_H__
#define __BASE_PREDRAW_CHECK_H__

#include "cards/card_effect.h"

namespace banggame {

    struct request_predraw : request_base {
        request_predraw(player *target)
            : request_base(nullptr, nullptr, target) {}

        bool auto_resolve() override { return auto_pick(); }
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };
    
    struct equip_predraw_check {
        int priority;
        equip_predraw_check(int priority) : priority(priority) {}

        void on_enable(card *target_card, player *target);
        void on_disable(card *target_card, player *target);
    };

}

#endif