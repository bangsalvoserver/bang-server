#ifndef __VALLEYOFSHADOWS_REQUESTS_H__
#define __VALLEYOFSHADOWS_REQUESTS_H__

#include "../card_effect.h"

#include "../base/bang.h"

namespace banggame {

    struct request_card_as_gatling : request_bang {
        using request_bang::request_bang;
        game_string status_text(player *owner) const override;
    };

    struct request_bandidos : request_base, resolvable_request {
        using request_base::request_base;

        int num_cards = 2;

        bool can_pick(pocket_type pocket, player *target, card *target_card) const override;
        void on_pick(pocket_type pocket, player *target, card *target_card) override;
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };

    struct request_tornado : request_base {
        using request_base::request_base;
        
        bool can_pick(pocket_type pocket, player *target, card *target_card) const override;
        void on_pick(pocket_type pocket, player *target, card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_poker : request_base {
        using request_base::request_base;

        bool can_pick(pocket_type pocket, player *target, card *target_card) const override;
        void on_pick(pocket_type pocket, player *target, card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_poker_draw : selection_picker {
        request_poker_draw(card *origin_card, player *target)
            : selection_picker(origin_card, nullptr, target) {}

        int num_cards = 2;

        void on_pick(pocket_type pocket, player *target, card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_saved : request_base {
        request_saved(card *origin_card, player *target, player *saved)
            : request_base(origin_card, nullptr, target, effect_flags::auto_pick)
            , saved(saved) {}

        player *saved = nullptr;

        bool can_pick(pocket_type pocket, player *target, card *target_card) const override;
        void on_pick(pocket_type pocket, player *target, card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_lemonade_jim : request_base {
        request_lemonade_jim(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target, effect_flags::auto_respond) {}
        
        game_string status_text(player *owner) const override;
    };

}

#endif