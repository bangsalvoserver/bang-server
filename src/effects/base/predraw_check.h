#ifndef __BASE_PREDRAW_CHECK_H__
#define __BASE_PREDRAW_CHECK_H__

#include "cards/card_effect.h"
#include "game/event_card_key.h"
#include "draw_check.h"

namespace banggame {

    namespace event_type {
        struct get_predraw_checks {
            player *origin;
            nullable_ref<std::vector<event_card_key>> result;
        };
        
        struct on_predraw_check {
            player *origin;
            card *target_card;
        };
    }

    struct request_predraw : request_picking {
        request_predraw(player *target)
            : request_picking(nullptr, nullptr, target, {}, -7) {}

        std::vector<event_card_key> checks;

        void on_update() override;
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

    DEFINE_EQUIP(predraw_check, equip_predraw_check)

}

#endif