#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "game/card_enums.h"
#include "game/game_string.h"
#include "game/durations.h"

#include <memory>

namespace banggame {

    template<target_type E> struct tagged_value {};

    template<target_type E>
    requires (play_card_target::has_type<E>)
    struct tagged_value<E> {
        typename play_card_target::value_type<E> value;
    };

    template<target_type E>
    using opt_tagged_value = std::optional<tagged_value<E>>;

    struct event_equip {
        void on_disable(card *target_card, player *target);
    };

    class request_base;

    class request_timer {
    protected:
        request_base *request;

    private:
        bool started = false;
        ticks duration;
        
        std::vector<player *> awaiting_confirms;
        ticks auto_confirm_timer = auto_confirm_duration;

    public:
        request_timer(request_base *request, ticks duration)
            : request(request), duration(std::clamp(duration, ticks{0}, max_timer_duration)) {}
            
        explicit request_timer(request_base *request);

    public:
        void add_pending_confirms();
        void confirm_player(player *p);

        void tick();

        virtual void on_finished() {}
    };

    class request_base {
    public:
        request_base(card *origin_card, player *origin, player *target, effect_flags flags = {})
            : origin_card(origin_card), origin(origin), target(target), flags(flags) {}
        
        virtual ~request_base() {}

        card *origin_card;
        player *origin;
        player *target;
        effect_flags flags;

        virtual request_timer *timer() { return nullptr; }

        virtual game_string status_text(player *owner) const = 0;

        virtual bool can_pick(card *target_card) const { return false; }
        virtual void on_pick(card *target_card);

        virtual bool can_respond(player *target, card *target_card) const;

        virtual void on_update() {}
        virtual bool auto_resolve() { return false; }

        virtual std::vector<card *> get_highlights() const {
            return {};
        }
    
    protected:
        bool auto_pick();
        bool auto_respond();
    };

    class cleanup_request {
    public:
        cleanup_request() = default;
        ~cleanup_request() {
            if (m_fun) {
                m_fun();
                m_fun = nullptr;
            }
        }

        cleanup_request(const cleanup_request &) = delete;
        cleanup_request(cleanup_request &&other) noexcept
            : m_fun(std::move(other.m_fun))
        {
            other.m_fun = nullptr;
        }

        cleanup_request &operator = (const cleanup_request &) = delete;
        cleanup_request &operator = (cleanup_request &&other) noexcept {
            m_fun = std::move(other.m_fun);
            other.m_fun = nullptr;
            return *this;
        }

        void on_cleanup(std::function<void()> &&fun) {
            m_fun = std::move(fun);
        }

    private:
        std::function<void()> m_fun;
    };

    struct selection_picker : request_base {
        using request_base::request_base;

        bool can_pick(card *target_card) const override;
    };

    struct resolvable_request {
        virtual void on_resolve() = 0;
    };

}


#endif