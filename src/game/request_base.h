#ifndef __REQUEST_BASE_H__
#define __REQUEST_BASE_H__

#include <memory>

#include "cards/game_string.h"
#include "net/options.h"

namespace banggame {

    class request_queue;
    class request_base;

    static constexpr ticks max_timer_duration = 10s;
    
    using timer_id_t = size_t;

    class request_timer {
    protected:
        ticks duration;

    private:
        timer_id_t timer_id;
        static inline timer_id_t timer_id_counter = 0;

        ticks lifetime{};

    public:
        template<typename Rep, typename Period>
        request_timer(std::chrono::duration<Rep, Period> duration)
            : duration{ std::chrono::duration_cast<ticks>(duration) }
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

        virtual void on_finished(request_base &request) {}
    };

    class request_base {
    public:
        request_base(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {}, int priority = 100)
            : origin_card(origin_card), origin(origin), target(target), flags(flags), priority(priority) {}
        
        virtual ~request_base() {}

        card_ptr origin_card;
        player_ptr origin;
        player_ptr target;
        effect_flags flags;
        int priority;
        int update_count = 0;

        virtual void on_update() {}

        virtual request_timer *timer() { return nullptr; }

        virtual game_string status_text(player_ptr owner) const { return {}; };
        virtual card_list get_highlights(player_ptr owner) const { return {}; }
    };

    struct interface_target_set_players {
        virtual bool in_target_set(const_player_ptr target_player) const = 0;
    };

    struct interface_target_set_cards {  
        virtual bool in_target_set(const_card_ptr target_card) const = 0;
    };

}

#endif