#include "request_queue.h"

#include "game_table.h"

#include "utils/type_name.h"

#include "net/logging.h"

namespace banggame {

    request_state request_queue::invoke_update() {
        if (is_game_over()) {
            return utils::tag<"done">{};
        } else if (auto req = top_request()) {
            logging::debug("on_update() on {: >5}: {}", req->priority, utils::demangle(typeid(*req).name()));

            req->on_update();
            req->live = true;
            
            if (top_request() != req) {
                return utils::tag<"next">{};
            }
            if (request_timer *timer = req->timer()) {
                if (timer->get_duration() <= ticks{0}) {
                    pop_request();
                    timer->on_finished();
                    return utils::tag<"next">{};
                }
                timer->start(get_total_update_time());
            }
            send_request_update();
        } else if (holds_alternative<"next">(send_request_status_ready())) {
            return utils::tag<"next">{};
        }
        return request_bot_play(false);
    }

    request_state request_queue::invoke_tick_update() {
        if (is_game_over()) {
            return utils::tag<"done">{};
        } else if (auto req = top_request()) {
            if (request_timer *timer = req->timer()) {
                timer->tick();
                if (timer->finished()) {
                    send_request_status_clear();
                    pop_request();
                    timer->on_finished();
                    return utils::tag<"next">{};
                }
            }
        }

        return utils::visit_tagged(overloaded{
            [](const auto &state) -> request_state { return state; },
            [](utils::tag<"waiting">, ticks timer) -> request_state {
                if (timer > ticks{}) {
                    return {utils::tag<"waiting">{}, timer - ticks{1} };
                } else {
                    return utils::tag<"next">{};
                }
            },
            [&](utils::tag<"bot_play">, ticks timer) -> request_state {
                if (timer > ticks{}) {
                    return {utils::tag<"bot_play">{}, timer - ticks{1} };
                } else {
                    return request_bot_play(true);
                }
            }
        }, m_state);
    }
    
    void request_queue::tick() {
        m_state = invoke_tick_update();

        if (holds_alternative<"next">(m_state)) {
            commit_updates();
        }
    }

    static constexpr ticks max_update_timer_duration = 10s;
    static constexpr int max_update_count = 30;
    
    void request_queue::commit_updates() {
        int count = 0;
        do {
            auto timer = get_total_update_time();
            if (timer > max_update_timer_duration || count > max_update_count) {
                m_state = { utils::tag<"waiting">{}, timer };
            } else {
                m_state = invoke_update();
                ++count;
            }
        } while (holds_alternative<"next">(m_state));
    }
}