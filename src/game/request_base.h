#ifndef __REQUEST_BASE_H__
#define __REQUEST_BASE_H__

#include <memory>

#include "cards/game_string.h"
#include "net/options.h"

namespace banggame {

    class request_base {
    public:
        request_base(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {}, int priority = 100)
            : origin_card(origin_card), origin(origin), target(target), flags(flags), priority(priority) {}
        
        virtual ~request_base() {}

        card_ptr origin_card;
        player_ptr origin;
        player_ptr target;
        effect_flags flags;
        int priority;
        int update_count = 0;

        virtual void on_update() {}

        virtual game_string status_text(player_ptr owner) const { return {}; };
        virtual card_list get_highlights(player_ptr owner) const { return {}; }
    };

    struct interface_target_set_players {
        virtual bool in_target_set(const_player_ptr target_player) const = 0;
    };

    struct interface_target_set_cards {  
        virtual bool in_target_set(const_card_ptr target_card) const = 0;
    };

}

#endif