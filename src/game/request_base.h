#ifndef __REQUEST_BASE_H__
#define __REQUEST_BASE_H__

#include <memory>

#include "cards/game_string.h"
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
        timer_id_t timer_id;
        static inline timer_id_t timer_id_counter = 0;

        ticks lifetime = max_timer_duration;

    public:
        request_timer(request_base *request, auto duration)
            : request{ request }
            , duration{ clamp_ticks(duration) }
            , timer_id{ timer_id_counter++ } {}

        timer_id_t get_timer_id() const {
            return timer_id;
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
    };

    struct interface_target_set {
        virtual bool in_target_set(const player *target_player) const {
            return false;
        }
        
        virtual bool in_target_set(const card *target_card) const {
            return false;
        }
    };

}

#endif