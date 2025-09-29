#ifndef __WILDWESTSHOW_YOUL_GRINNER__
#define __WILDWESTSHOW_YOUL_GRINNER__

#include "cards/card_effect.h"
#include "effects/base/pick.h"

namespace banggame {

    struct request_youl_grinner : request_picking {
        request_youl_grinner(card_ptr origin_card, player_ptr origin, player_ptr target)
            : request_picking(origin_card, origin, target) {}

        void on_update() override {
            auto_pick();
        }

        bool can_pick(const_card_ptr target_card) const override;
        prompt_string pick_prompt(card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };

    struct equip_youl_grinner : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(youl_grinner, equip_youl_grinner)
}

#endif