#ifndef __REQUEST_BASE_H__
#define __REQUEST_BASE_H__

#include <memory>

#include "game_string.h"
#include "net/options.h"

namespace banggame {

    class request_queue;
    class request_base;

    static constexpr ticks max_timer_duration = 10s;

    template<typename Rep, typename Period>
    inline ticks clamp_ticks(std::chrono::duration<Rep, Period> duration) {
        return std::clamp(std::chrono::duration_cast<ticks>(duration), ticks{0}, max_timer_duration);
    }
    
    using timer_id_t = size_t;

    class request_timer {
    protected:
        request_base *request;
        ticks duration;

    private:
        ticks lifetime = max_timer_duration;

    public:
        request_timer(request_base *request, auto duration)
            : request(request), duration(clamp_ticks(duration)) {}

        timer_id_t get_timer_id() const {
            return std::hash<const request_timer *>{}(this);
        }

        ticks get_duration() const {
            return duration;
        }

        void start(ticks total_update_time) {
            lifetime = duration + total_update_time;
        }

        void tick() {
            --lifetime;
        }

        bool finished() const {
            return lifetime <= ticks{0};
        }

        virtual void on_finished() {}
    };

    class request_base {
    public:
        request_base(card *origin_card, player *origin, player *target, effect_flags flags = {}, int priority = 100)
            : origin_card(origin_card), origin(origin), target(target), flags(flags), priority(priority) {}
        
        virtual ~request_base() {}

        card *origin_card;
        player *origin;
        player *target;
        effect_flags flags;
        int priority;
        bool live = false;

        virtual void on_update() {}

        virtual request_timer *timer() { return nullptr; }

        virtual game_string status_text(player *owner) const { return {}; };
        virtual std::vector<card *> get_highlights() const { return {}; }
        virtual std::vector<player *> get_target_set() const { return {}; }
    
    protected:
        void auto_respond();
    };

    struct request_picking_base {
        virtual bool can_pick(card *target_card) const = 0;
        virtual void on_pick(card *target_card) = 0;
        virtual game_string pick_prompt(card *target_card) const { return {}; }
    };

    class request_picking : public request_base, public request_picking_base {
    public:
        using request_base::request_base;

    protected:
        void auto_pick();
    };

    struct selection_picker : request_picking {
        using request_picking::request_picking;

        bool can_pick(card *target_card) const override;
    };

    struct resolvable_request {
        virtual void on_resolve() = 0;
    };

}

#endif