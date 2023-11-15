#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <deque>
#include <memory>
#include <concepts>
#include <functional>

#include "cards/card_effect.h"

#include "utils/stable_queue.h"

namespace banggame {

    using delayed_action = std::function<void()>;

    using action_priority_pair = std::pair<delayed_action, int>;

    struct action_ordering {
        bool operator()(const action_priority_pair &lhs, const action_priority_pair &rhs) const {
            return lhs.second < rhs.second;
        }
    };

    struct game;

    struct update_done {};
    struct update_next {};
    struct update_waiting { ticks timer; };
    struct update_bot_play { ticks timer; };

    using request_update_state = std::variant<update_done, update_next, update_waiting, update_bot_play>;

    class request_queue {
    private:
        std::deque<std::shared_ptr<request_base>> m_requests;
        utils::stable_priority_queue<action_priority_pair, action_ordering> m_delayed_actions;

        game *m_game;

        request_update_state m_state;

        request_update_state invoke_update();
        request_update_state invoke_tick_update();

    public:
        request_queue(game *m_game) : m_game(m_game) {}
        
        void tick();
        void commit_updates();

    public:
        bool pending_requests() const {
            return !m_requests.empty();
        }

        bool pending_updates() const {
            return std::holds_alternative<update_waiting>(m_state);
        }

        template<typename T = request_base>
        std::shared_ptr<T> top_request(player *target = nullptr) {
            if (!m_requests.empty()) {
                auto req = m_requests.front();
                if (!target || req->target == target) {
                    return std::dynamic_pointer_cast<T>(req);
                }
            }
            return nullptr;
        }

        void queue_action(delayed_action &&fun, int priority = 0) {
            m_delayed_actions.emplace(std::move(fun), priority);
        }

        void queue_request(std::shared_ptr<request_base> &&value) {
            m_requests.emplace_back(std::move(value));
        }

        template<std::derived_from<request_base> T>
        void queue_request(auto && ... args) {
            queue_request(std::make_shared<T>(FWD(args) ... ));
        }

        void queue_request_front(std::shared_ptr<request_base> &&value) {
            m_requests.emplace_front(std::move(value));
        }

        template<std::derived_from<request_base> T>
        void queue_request_front(auto && ... args) {
            queue_request_front(std::make_shared<T>(FWD(args) ... ));
        }

        void pop_request() {
            top_request()->state = request_state::dead;
            m_requests.pop_front();
        }
    };

}

#endif