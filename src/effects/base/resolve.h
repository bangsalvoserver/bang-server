#ifndef __BASE_RESOLVE_H__
#define __BASE_RESOLVE_H__

#include "cards/card_effect.h"

namespace banggame {
    struct effect_resolve {
        int resolve_type;
        effect_resolve(int resolve_type): resolve_type{resolve_type} {}

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin);
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(resolve, effect_resolve)

    struct interface_resolvable {
        virtual void on_resolve() = 0;
        virtual int resolve_type() const { return 0; }
        virtual prompt_string resolve_prompt() const { return {}; }
    };

    class request_resolvable: public request_base, public interface_resolvable {
    public:
        using request_base::request_base;

    protected:
        void auto_resolve();
    };
}

#endif