#ifndef __BASE_REQUESTS_H__
#define __BASE_REQUESTS_H__

#include "cards/card_effect.h"
#include "pick.h"
#include "resolve.h"

namespace banggame {

    struct request_characterchoice : request_picking {
        request_characterchoice(player_ptr target)
            : request_picking(nullptr, nullptr, target) {}
        
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };

    struct request_character_modifier : request_base {
        request_character_modifier(player_ptr target)
            : request_base(nullptr, nullptr, target, {}, -25) {}

        std::set<card_ptr> handlers;
        
        void on_update() override;
    };
    
    struct request_discard : request_picking {
        using request_picking::request_picking;
        
        void on_update() override;
        prompt_string pick_prompt(card_ptr target_card) const override;
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };

    namespace event_type {
        struct on_discard_pass {
            player_ptr origin;
            card_ptr target_card;
        };

        struct on_discard_all {
            player_ptr origin;
        };
        
        struct post_discard_pass {
            player_ptr origin;
            int ndiscarded;
        };
    }

    struct request_discard_pass : request_picking {
        request_discard_pass(player_ptr target)
            : request_picking(nullptr, nullptr, target, {}, 200) {}

        int ndiscarded = 0;

        void on_update() override;

        prompt_string pick_prompt(card_ptr target_card) const override;
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        game_string status_text(player_ptr owner) const override;
    };

    struct request_discard_all : request_picking, interface_resolvable {
        request_discard_all(player_ptr target, int priority = 100)
            : request_picking(nullptr, nullptr, target, {}, priority) {}
        
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        void on_update() override;
        void on_resolve() override;
        game_string status_text(player_ptr owner) const override;
    };

    struct request_sheriff_killed_deputy : request_discard_all {
        using request_discard_all::request_discard_all;

        game_string status_text(player_ptr owner) const override;
    };
    
    struct request_discard_hand : request_picking, interface_resolvable {
        request_discard_hand(card_ptr origin_card, player_ptr target)
            : request_picking(origin_card, nullptr, target) {}
        
        bool can_pick(const_card_ptr target_card) const override;
        void on_pick(card_ptr target_card) override;
        void on_update() override;
        void on_resolve() override;
        game_string status_text(player_ptr owner) const override;
    };

}

#endif