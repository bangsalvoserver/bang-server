#include "request_queue.h"

#include "game.h"

namespace banggame {
    
    void request_queue::update_request() {
        if (m_lock_updates) return;

        if (pending_requests()) {
            auto &req = top_request();
            auto weak_ptr = std::weak_ptr(req.ptr());
            req.on_update();
            if (!weak_ptr.expired()) {
                req.start(m_game->get_total_update_time());
                m_game->send_request_update();

                for (player *p : m_game->m_players) {
                    m_game->request_bot_play(p, true);
                    if (weak_ptr.expired()) break;
                }
            }
        } else if (!m_delayed_actions.empty()) {
            auto lock = lock_updates();
            auto fun = std::move(m_delayed_actions.top().first);
            m_delayed_actions.pop();
            std::invoke(fun);
        } else if (m_game->m_playing) {
            m_game->request_bot_play(m_game->m_playing, false);
        }
    }

    request_queue::update_lock_guard::~update_lock_guard() noexcept(false) {
        copy.reset();
        if (self) {
            --self->m_lock_updates;
            std::exchange(self, nullptr)->update_request();
        }
    }

    request_queue::update_lock_guard request_queue::lock_updates(bool pop) {
        ++m_lock_updates;
        std::shared_ptr<request_base> copy;
        if (pending_requests()) {
            copy = top_request().ptr();
        }
        if (pop) {
            pop_request();
        }
        return update_lock_guard{this, std::move(copy)};
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
            m_game->send_request_status_clear();
        }
        m_requests.pop_front();
        update_request();
    }
}