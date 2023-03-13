#include "request_queue.h"

#include "game.h"

namespace banggame {
    
    void request_queue::tick() {
        if (auto req = top_request()) {
            if (auto *timer = req->timer()) {
                timer->tick();
                if (timer->finished()) {
                    m_game->send_request_status_clear();
                    invoke_action([&]{
                        pop_request();
                        timer->on_finished();
                        req.reset();
                    });
                }
            }
        }
    }
    
    void request_queue::update_request() {
        if (m_lock_updates) return;
        if (m_game->is_game_over()) return;

        if (auto req = top_request()) {
            ++m_lock_updates;
            req->on_update();
            --m_lock_updates;

            if (req->state == request_state::dead) {
                req.reset();
                update_request();
            } else {
                req->state = request_state::live;
                if (auto *timer = req->timer()) {
                    timer->start(m_game->get_total_update_time());
                }
                m_game->send_request_update();
                req.reset();

                for (player *p : m_game->m_players) {
                    if (p->is_bot() && m_game->request_bot_play(p, true)) {
                        break;
                    }
                }
            }
        } else if (!m_delayed_actions.empty()) {
            auto fun = std::move(m_delayed_actions.top().first);
            m_delayed_actions.pop();
            invoke_action(std::move(fun));
        } else if (m_game->m_playing) {
            if (m_game->m_playing->is_bot()) {
                m_game->request_bot_play(m_game->m_playing, false);
            } else {
                m_game->send_request_status_ready();
            }
        }
    }
}