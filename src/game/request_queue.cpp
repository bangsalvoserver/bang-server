#include "request_queue.h"

#include "game.h"

namespace banggame {
    
    void request_queue::update_request() {
        if (m_lock_updates) return;

        if (pending_requests()) {
            auto &req = top_request();
            auto weak_ptr = req.weak_ptr();
            req.on_update();
            if (!weak_ptr.expired()) {
                req.start();
                req.target()->m_game->send_request_update();
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
        if (!locked()) {
            std::invoke(fun);
        } else {
            m_delayed_actions.emplace(std::move(fun), priority);
        }
    }

    void request_queue::pop_request() {
        auto &req = top_request();
        if (req.is_sent()) {
            req.target()->m_game->send_request_status_clear();
        }
        m_requests.pop_front();
        update_request();
    }
}