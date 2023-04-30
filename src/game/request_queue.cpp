#include "request_queue.h"

#include "game.h"
#include "bot_ai.h"

namespace banggame {
    
    static ticks get_total_update_time(game *game) {
        return ranges::max(game->m_players | ranges::views::transform([&](player *p) {
            return ranges::accumulate(game->m_updates | ranges::views::transform([&](const game_update_tuple &tup) {
                if (tup.duration >= ticks{0} && tup.target.matches(p)) {
                    return tup.duration;
                }
                return ticks{0};
            }), ticks{0});
        }));
    }

    static ticks get_bot_play_timer(game *game) {
        return get_total_update_time(game) + clamp_ticks(game->m_options.bot_play_timer);
    }

    void request_queue::invoke_update() {
        if (m_game->is_game_over()) return;

        if (auto req = top_request()) {
            if (m_bot_play) {
                m_bot_play = false;
                req.reset();
                
                for (player *origin : m_game->m_players) {
                    if (origin->is_bot() && bot_ai::respond_to_request(origin)) {
                        break;
                    }
                }
            } else {
                req->on_update();

                if (req->state == request_state::pending) {
                    req->state = request_state::live;
                }

                if (top_request() == req) {
                    if (auto *timer = req->timer()) {
                        timer->start(get_total_update_time(m_game));
                    }
                    m_game->send_request_update();
                    if (std::ranges::any_of(m_game->m_players, &player::is_bot)) {
                        m_update_timer = get_bot_play_timer(m_game);
                        m_bot_play = true;
                    }
                } else {
                    req.reset();
                    update();
                }
            }
        } else if (!m_delayed_actions.empty()) {
            auto fun = std::move(m_delayed_actions.top().first);
            m_delayed_actions.pop();
            std::invoke(std::move(fun));
            update();
        } else if (player *origin = m_game->m_playing) {
            if (m_bot_play) {
                m_bot_play = false;
                bot_ai::play_in_turn(origin);
            } else if (m_game->send_request_status_ready() && origin->is_bot()) {
                m_update_timer = get_bot_play_timer(m_game);
                m_bot_play = true;
            }
        }
    }
    
    void request_queue::tick() {
        if (m_game->is_game_over()) return;

        if (m_update_timer && --(*m_update_timer) <= ticks{}) {
            m_update_timer.reset();
            invoke_update();
        } else if (auto req = top_request()) {
            if (auto *timer = req->timer()) {
                timer->tick();
                if (timer->finished()) {
                    m_game->send_request_status_clear();
                    pop_request();
                    timer->on_finished();
                    req.reset();
                    update();
                }
            }
        }
    }
    
    void request_queue::update() {
        m_bot_play = false;
        auto timer = get_total_update_time(m_game);
        if (timer < clamp_ticks(max_timer_duration)) {
            m_update_timer.reset();
            invoke_update();
        } else {
            m_update_timer = timer;
        }
    }
}