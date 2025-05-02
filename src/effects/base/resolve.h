#ifndef __BASE_RESOLVE_H__
#define __BASE_RESOLVE_H__

#include "cards/card_effect.h"

#include "game/request_timer.h"

namespace banggame {

    enum class resolve_type {
        resolve,
        dismiss
    };

    struct effect_resolve {
        resolve_type type;
        effect_resolve(resolve_type type): type{type} {}

        prompt_string on_prompt(card_ptr origin_card, player_ptr origin);
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(resolve, effect_resolve)

    struct interface_resolvable {
        virtual void on_resolve() = 0;
        virtual resolve_type get_resolve_type() const { return resolve_type::resolve; }
        virtual prompt_string resolve_prompt() const { return {}; }
    };

    class request_resolvable: public request_base, public interface_resolvable {
    public:
        using request_base::request_base;

    protected:
        bool auto_resolvable() const;
        void auto_resolve();
    };

    struct request_resolvable_timer : request_resolvable, request_timer {
        using request_resolvable::request_resolvable;

        void on_finished() override {
            on_resolve();
        }
    };

    class request_auto_resolvable: public request_resolvable_timer {
    public:
        using request_resolvable_timer::request_resolvable_timer;
    
    protected:
        void auto_resolve();
    };
}

#endif