#ifndef __CARD_EFFECT_H__
#define __CARD_EFFECT_H__

#include "card_enums.h"

#include "game/game_string.h"

#include "net/options.h"

#include <memory>

namespace banggame {

    struct effect_holder;

    struct effect_target_pair {
        const effect_holder &effect;
        play_card_target target;
    };
    
    using effect_target_list = std::vector<effect_target_pair>;

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

    class request_queue;
    class request_base;

    static constexpr ticks max_timer_duration = 10s;

    class request_timer {
    protected:
        request_base *request;
        ticks duration;

    private:
        ticks lifetime = max_timer_duration;

    public:
        template<typename Duration>
        request_timer(request_base *request, Duration duration)
            : request(request)
            , duration(std::clamp(std::chrono::duration_cast<ticks>(duration), ticks{0}, max_timer_duration)) {}

    public:
        void start(ticks total_update_time);
        void tick();
        bool finished() const { return lifetime <= ticks{0}; }
        virtual void on_finished() {}

        void disable() { request = nullptr; }
        bool enabled() const { return request != nullptr; }
    };

    enum class request_state : uint8_t {
        pending,
        live,
        dead
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

        request_state state = request_state::pending;

        virtual request_timer *timer() { return nullptr; }

        virtual game_string status_text(player *owner) const { return {}; };

        virtual bool auto_select() const { return false; }

        virtual bool can_pick(card *target_card) const { return false; }
        virtual void on_pick(card *target_card) { throw std::runtime_error("missing on_pick(card)"); }

        virtual void on_update() {}

        virtual std::vector<card *> get_highlights() const { return {}; }
    
    protected:
        void auto_pick();
        void auto_respond();
    };

    struct selection_picker : request_base {
        using request_base::request_base;

        bool can_pick(card *target_card) const override;
    };

    struct request_auto_select : request_base {
        using request_base::request_base;

        bool auto_select() const override { return true; }
    };

    struct resolvable_request {
        virtual void on_resolve() = 0;
    };

    struct effect_context;

}


#endif