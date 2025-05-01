#ifndef __REQUEST_TIMER_H__
#define __REQUEST_TIMER_H__

#include "game_update.h"

#include "net/options.h"

namespace banggame {
    
    class request_timer {
    private:
        static inline timer_id_t timer_id_counter = 0;
        
        timer_id_t timer_id = 0;
        game_duration duration{-1};
        ticks lifetime{0};

    public:
        timer_id_t get_timer_id() const {
            return timer_id;
        }

        void set_duration(game_duration value) {
            duration = value;
        }

        game_duration get_duration() const {
            return duration;
        }

        bool enabled() const {
            return duration >= game_duration{0};
        }

        void start(ticks total_update_time) {
            lifetime = std::chrono::duration_cast<ticks>(duration) + total_update_time;
            timer_id = timer_id_counter++;
        }

        void tick() {
            --lifetime;
        }

        bool finished() const {
            return lifetime <= ticks{0};
        }

        virtual void on_finished() = 0;
    };

}

#endif