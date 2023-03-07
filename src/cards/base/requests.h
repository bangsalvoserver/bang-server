#ifndef __BASE_REQUESTS_H__
#define __BASE_REQUESTS_H__

#include "cards/card_effect.h"

namespace banggame {

    struct request_characterchoice : request_base {
        request_characterchoice(player *target)
            : request_base(nullptr, nullptr, target) {}
        
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };
    
    struct request_discard : request_base {
        request_discard(card *origin_card, player *origin, player *target, int ncards = 1)
            : request_base(origin_card, origin, target)
            , ncards(ncards) {}

        int ncards;
        
        void on_update() override { auto_pick(); }
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_discard_pass : request_base {
        request_discard_pass(player *target)
            : request_base(nullptr, nullptr, target) {}

        int ndiscarded = 0;

        void on_update() override;

        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_discard_all : request_base, resolvable_request {
        discard_all_reason reason;

        request_discard_all(player *target, discard_all_reason reason)
            : request_base(nullptr, nullptr, target)
            , reason(reason) {}
        
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        void on_update() override;
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };
    
    struct request_discard_hand : request_base, resolvable_request {
        request_discard_hand(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target) {}
        
        bool can_pick(card *target_card) const override;
        void on_pick(card *target_card) override;
        void on_update() override;
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };

}

#endif