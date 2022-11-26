#include "request_queue.h"

namespace banggame {
    request_queue::update_lock_guard::~update_lock_guard() noexcept(false) {
        if (self) {
            --self->m_lock_updates;
            std::exchange(self, nullptr)->update_request();
        }
    }

    void request_queue::update_actions() {
        while (!pending_requests() && !m_delayed_actions.empty() && !m_lock_updates) {
            auto fun = std::move(m_delayed_actions.front());
            m_delayed_actions.pop_front();
            std::invoke(fun);
        }
    }
    
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
            update_actions();
        }
    }

    request_queue::update_lock_guard request_queue::lock_updates(bool pop) {
        ++m_lock_updates;
        if (pop) {
            pop_request();
        }
        return update_lock_guard{this};
    }
    
    void request_queue::queue_action(delayed_action &&fun) {
        m_delayed_actions.emplace_back(std::move(fun));
        update_actions();
    }

    void request_queue::queue_action_front(delayed_action &&fun) {
        m_delayed_actions.emplace_front(std::move(fun));
        update_actions();
    }
}