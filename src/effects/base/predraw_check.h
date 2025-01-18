#ifndef __BASE_PREDRAW_CHECK_H__
#define __BASE_PREDRAW_CHECK_H__

#include "cards/card_effect.h"
#include "game/event_card_key.h"
#include "draw_check.h"

namespace banggame {

    namespace event_type {
        struct get_predraw_checks {
            player_ptr origin;
            nullable_ref<std::vector<event_card_key>> result;
        };
        
        struct on_predraw_check {
            player_ptr origin;
            card_ptr target_card;
        };

        struct check_predraw_auto_pick {
            player_ptr origin;
            card_ptr checking_card;
            nullable_ref<bool> value;
        };
    }

    struct request_predraw : request_picking {
        request_predraw(player_ptr target)
            : request_picking(nullptr, nullptr, target, {}, -20) {}

        std::vector<event_card_key> checks;

        std::span<const event_card_key> get_checking_cards() const;
        void remove_check(card_ptr target_card);
        
        void on_update() override;
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };
    
    struct equip_predraw_check {
        int priority;
        equip_predraw_check(int priority) : priority(priority) {}

        void on_enable(card_ptr target_card, player_ptr target);
        void on_disable(card_ptr target_card, player_ptr target);
    };

    DEFINE_EQUIP(predraw_check, equip_predraw_check)

}

#endif