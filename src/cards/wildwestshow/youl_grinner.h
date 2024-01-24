#ifndef __WILDWESTSHOW_YOUL_GRINNER__
#define __WILDWESTSHOW_YOUL_GRINNER__

#include "cards/card_effect.h"

namespace banggame {

    struct request_youl_grinner : request_picking {
        request_youl_grinner(card *origin_card, player *origin, player *target)
            : request_picking(origin_card, origin, target) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct equip_youl_grinner : event_equip {
        void on_enable(card *target_card, player *target);
    };
}

#endif