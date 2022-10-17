#ifndef __BASE_BANG_H__
#define __BASE_BANG_H__

#include "../card_effect.h"

namespace banggame {
    
    struct effect_bang {
        void on_play(card *origin_card, player *origin, player *target, effect_flags flags = {});
    };

    struct handler_bangcard {
        void on_play(card *origin_card, player *origin, player *target);
    };

    struct handler_play_as_bang {
        void on_play(card *origin_card, player *origin, card *chosen_card, player *target);
    };

    struct effect_banglimit {
        game_string verify(card *origin_card, player *origin);
        void on_play(card *origin_card, player *origin);
    };

    class missable_request {
    public:
        void add_card(card *c) {
            m_cards_used.push_back(c);
        }

        virtual bool can_miss(card *c) const {
            return std::ranges::find(m_cards_used, c) == m_cards_used.end();
        }

        virtual void on_miss() = 0;

    private:
        std::vector<card *> m_cards_used;
    };

    struct request_bang : request_base, missable_request, cleanup_request, resolvable_request {
        using request_base::request_base;

        int bang_strength = 1;
        int bang_damage = 1;
        bool unavoidable = false;

        bool can_miss(card *c) const override;

        void on_miss() override;
        void on_resolve() override;

        void set_unavoidable();

        game_string status_text(player *owner) const override;
    };

}

#endif