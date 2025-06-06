#include "request_queue.h"

#include "game_table.h"

#include "request_timer.h"

#include "utils/type_name.h"

#include "net/logging.h"

namespace banggame {

    request_state request_queue::invoke_update() {
        if (is_game_over()) {
            return request_states::done{};
        } else if (auto req = top_request()) {
            logging::debug("on_update() on {: >5}: {}", req->priority, typeid(*req));

            req->on_update();
            ++req->update_count;
            
            if (top_request() != req) {
                return request_states::next{};
            }
            if (auto *timer = dynamic_cast<request_timer *>(req.get()); timer && timer->enabled()) {
                if (timer->get_duration() <= game_duration{0}) {
                    timer->on_finished();
                    return request_states::next{};
                }
                timer->start(get_total_update_time());
            }
            send_request_update();
        } else if (std::holds_alternative<request_states::next>(send_request_status_ready())) {
            return request_states::next{};
        }
        return request_bot_play(false);
    }

    request_state request_queue::invoke_tick_update() {
        if (is_game_over()) {
            return request_states::done{};
        } else if (auto timer = top_request<request_timer>(); timer && timer->enabled()) {
            timer->tick();
            if (timer->finished()) {
                send_request_status_clear();
                timer->on_finished();
                return request_states::next{};
            }
        }

        return std::visit(overloaded{
            [](auto state) -> request_state { return state; },
            [](request_states::waiting state) -> request_state {
                if (state.timer > ticks{}) {
                    return request_states::waiting{ state.timer - ticks{1} };
                } else {
                    return request_states::next{};
                }
            },
            [&](request_states::bot_play state) -> request_state {
                if (state.timer > ticks{}) {
                    return request_states::bot_play{ state.timer - ticks{1} };
                } else {
                    return request_bot_play(true);
                }
            }
        }, m_state);
    }
    
    void request_queue::tick() {
        m_state = invoke_tick_update();

        if (std::holds_alternative<request_states::next>(m_state)) {
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
                m_state = request_states::waiting{ timer };
            } else {
                m_state = invoke_update();
                ++count;
            }
        } while (std::holds_alternative<request_states::next>(m_state));
    }
}