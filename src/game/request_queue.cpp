#include "request_queue.h"

namespace banggame {
    
    void request_queue::update_request() {
        if (m_lock_updates) return;

        if (pending_requests()) {
            auto &req = top_request();
            req.on_update();
            if (!req.auto_resolve()) {
                req.add_pending_confirms();
                send_request_update();
            }
        } else {
            while (!pending_requests() && !m_delayed_actions.empty() && !m_lock_updates) {
                auto [fun, priority] = std::move(m_delayed_actions.top());
                m_delayed_actions.pop();
                std::invoke(fun);
            }
        }
    }

    request_queue::update_lock_guard::~update_lock_guard() noexcept(false) {
        if (self) {
            --self->m_lock_updates;
            std::exchange(self, nullptr)->update_request();
        }
    }

    request_queue::update_lock_guard request_queue::lock_updates(bool pop) {
        ++m_lock_updates;
        if (pop) {
            pop_request();
        }
        return update_lock_guard{this};
    }
    
    void request_queue::queue_action(delayed_action &&fun, int priority) {
        if (!pending_requests() && m_delayed_actions.empty() && !m_lock_updates) {
            std::invoke(fun);
        } else {
            m_delayed_actions.emplace(std::move(fun), priority);
        }
    }
}