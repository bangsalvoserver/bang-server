#include "request_queue.h"

#include "game.h"

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

    request_update_state request_queue::invoke_update() {
        if (m_game->is_game_over()) {
            return update_done{};
        } else if (auto req = top_request()) {
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
                    return update_bot_play{ get_bot_play_timer(m_game) };
                }
            } else {
                return update_next{};
            }
        } else if (!m_delayed_actions.empty()) {
            auto fun = std::move(m_delayed_actions.top().first);
            m_delayed_actions.pop();
            std::invoke(std::move(fun));
            return update_next{};
        } else if (player *origin = m_game->m_playing) {
            if (!origin->alive()) {
                m_game->start_next_turn();
                return update_next{};
            } else if (!m_game->send_request_status_ready()) {
                return update_next{};
            } else if (origin->is_bot()) {
                return update_bot_play{ get_bot_play_timer(m_game) };
            }
        }
        return update_done{};
    }

    request_update_state request_queue::invoke_tick_update() {
        if (m_game->is_game_over()) {
            return update_done{};
        } else if (auto req = top_request()) {
            if (auto *timer = req->timer()) {
                timer->tick();
                if (timer->finished()) {
                    m_game->send_request_status_clear();
                    pop_request();
                    timer->on_finished();
                    return update_next{};
                }
            }
        }

        return std::visit(overloaded{
            [](const auto &state) -> request_update_state { return state; },
            [](const update_waiting &state) -> request_update_state {
                if (state.timer > ticks{}) {
                    return update_waiting{ state.timer - ticks{1} };
                } else {
                    return update_next{};
                }
            },
            [&](const update_bot_play &state) -> request_update_state {
                if (state.timer > ticks{}) {
                    return update_bot_play{ state.timer - ticks{1} };
                } else if (m_game->request_bot_play()) {
                    return update_next{};
                } else {
                    return update_done{};
                }
            }
        }, m_state);
    }
    
    void request_queue::tick() {
        m_state = invoke_tick_update();

        if (std::holds_alternative<update_next>(m_state)) {
            commit_updates();
        }
    }

    static constexpr ticks max_update_timer_duration = 10s;
    
    void request_queue::commit_updates() {
        do {
            auto timer = get_total_update_time(m_game);
            if (timer > max_update_timer_duration) {
                m_state = update_waiting{ timer };
            } else {
                m_state = invoke_update();
            }
        } while (std::holds_alternative<update_next>(m_state));
    }
}