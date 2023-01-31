#include "request_queue.h"

#include "game.h"

namespace banggame {
    
    void request_queue::update_request() {
        if (m_lock_updates) return;
        if (m_game->check_flags(game_flags::game_over)) return;

        if (pending_requests()) {
            auto req = top_request();

            ++m_lock_updates;
            req.on_update();
            --m_lock_updates;

            if (req.is_popped()) {
                update_request();
            } else {
                req.start(m_game->get_total_update_time());
                m_game->send_request_update();

                for (player *p : m_game->m_players) {
                    if (p->is_bot() && m_game->request_bot_play(p, true)) {
                        break;
                    }
                }
            }
        } else if (!m_delayed_actions.empty()) {
            ++m_lock_updates;
            auto fun = std::move(m_delayed_actions.top().first);
            m_delayed_actions.pop();
            std::invoke(fun);
            --m_lock_updates;
            update_request();
        } else {
            m_game->send_request_status_ready();
        }
    }

    void request_queue::invoke_action(delayed_action &&fun) {
        {
            request_holder copy;
            if (pending_requests()) copy = top_request();
            ++m_lock_updates;
            std::invoke(fun);
            --m_lock_updates;
        }
        update_request();
    }
    
    void request_queue::queue_action(delayed_action &&fun, int priority) {
        m_delayed_actions.emplace(std::move(fun), priority);
        if (!pending_requests()) {
            update_request();
        }
    }

    void request_queue::pop_request() {
        auto &req = top_request();
        req.set_popped();
        if (req.is_sent()) {
            m_game->send_request_status_clear();
        }
        m_requests.pop_front();
        update_request();
    }
}