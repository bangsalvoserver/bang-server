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
    
    template<typename Function = decltype(ranges::max)>
    static ticks get_total_update_time(game *game, Function operation = {}) {
        return operation(game->m_players | ranges::views::transform([&](player *p) {
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
    
    void request_queue::tick() {
        if (m_game->is_game_over()) return;

        if (m_update_timer && --(*m_update_timer) <= ticks{}) {
            m_update_timer.reset();
            if (auto req = top_request()) {
                if (m_bot_play) {
                    m_bot_play = false;
                    for (player *p : m_game->m_players) {
                        if (p->is_bot() && bot_ai::respond_to_request(p)) {
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
                    bot_ai::play_in_turn(origin);
                } else {
                    m_game->send_request_status_ready();
                    if (!m_update_timer && origin->is_bot()) {
                        m_update_timer = get_bot_play_timer(m_game);
                        m_bot_play = true;
                    }
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
        m_bot_play = false;
    }
}