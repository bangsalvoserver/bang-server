#include "request_queue.h"

#include "game.h"
#include "bot_ai.h"

namespace banggame {

    struct average_fun {
        template<ranges::sized_range Range>
        auto operator()(Range &&range) const {
            return ranges::accumulate(range, ranges::range_value_t<Range>{}) / ranges::size(range);
        }
    };

    struct median_fun {
        template<ranges::range Range>
        auto operator()(Range &&range) const {
            auto [min, max] = ranges::minmax(range);
            return (min + max) / 2;
        }
    };
    
    template<typename Function = median_fun>
    static ticks get_total_update_time(game *game, Function operation = {}) {
        return operation(game->m_players | ranges::views::transform([&](player *p) {
            return ranges::accumulate(game->m_updates | ranges::views::transform([&](const game_update_tuple &tup) {
                if (tup.duration >= ticks{0} && tup.target.matches(p->user_id)) {
                    return tup.duration;
                }
                return ticks{0};
            }), ticks{0});
        }));
    }
    
    void request_queue::tick() {
        if (m_game->is_game_over()) return;

        if (m_update_timer && --(*m_update_timer) <= ticks{}) {
            m_update_timer.reset();
            if (auto req = top_request()) {
                req->on_update();

                if (req->state == request_state::dead) {
                    update();
                } else {
                    req->state = request_state::live;
                    if (auto *timer = req->timer()) {
                        timer->start(get_total_update_time(m_game));
                    }
                    m_game->send_request_update();

                    for (player *p : m_game->m_players) {
                        if (p->is_bot() && bot_ai::respond_to_request(p)) {
                            break;
                        }
                    }
                }
            } else if (!m_delayed_actions.empty()) {
                auto fun = std::move(m_delayed_actions.top().first);
                m_delayed_actions.pop();
                std::invoke(std::move(fun));
                update();
            } else if (m_game->m_playing) {
                if (m_game->m_playing->is_bot()) {
                    bot_ai::play_in_turn(m_game->m_playing);
                } else {
                    m_game->send_request_status_ready();
                }
            }
        } else if (auto req = top_request()) {
            if (auto *timer = req->timer()) {
                timer->tick();
                if (timer->finished()) {
                    m_game->send_request_status_clear();
                    pop_request();
                    timer->on_finished();
                    update();
                }
            }
        }
    }
    
    void request_queue::update() {
        m_update_timer = get_total_update_time(m_game);
    }
}