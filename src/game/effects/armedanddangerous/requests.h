#ifndef __ARMEDANDDANGEROUS_REQUESTS_H__
#define __ARMEDANDDANGEROUS_REQUESTS_H__

#include "../card_effect.h"

namespace banggame {

    struct request_add_cube : request_base {
        request_add_cube(card *origin_card, player *target, int ncubes = 1)
            : request_base(origin_card, nullptr, target)
            , ncubes(ncubes) {}

        int ncubes = 1;
        
        bool can_pick(pocket_type pocket, player *target, card *target_card) const override;
        void on_pick(pocket_type pocket, player *target, card *target_card) override;
        game_string status_text(player *owner) const override;
    };

    struct request_move_bomb : request_base {
        request_move_bomb(card *origin_card, player *target)
            : request_base(origin_card, nullptr, target, effect_flags::auto_respond) {}

        game_string status_text(player *owner) const override;
    };

    struct request_rust : request_base, resolvable_request {
        using request_base::request_base;

        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };

    struct request_al_preacher : request_base {
        request_al_preacher(card *origin_card, player *origin, player *target)
            : request_base(origin_card, origin, target, effect_flags::auto_respond) {}

        game_string status_text(player *owner) const override;
    };

    struct request_tumbleweed : request_base, resolvable_request {
        request_tumbleweed(card *origin_card, player *origin, player *target, card *drawn_card, card *drawing_card)
            : request_base(origin_card, origin, target)
            , target_card(drawing_card)
            , drawn_card(drawn_card) {}
        
        card *target_card;
        card *drawn_card;

        std::vector<card *> get_highlights() const override;
        void on_resolve() override;
        game_string status_text(player *owner) const override;
    };
}

#endif