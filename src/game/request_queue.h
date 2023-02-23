#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <deque>
#include <memory>
#include <concepts>
#include <functional>

#include "holders.h"
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

    class request_queue {
    private:
        std::deque<std::shared_ptr<request_base>> m_requests;
        utils::stable_priority_queue<action_priority_pair, action_ordering> m_delayed_actions;
        int m_lock_updates = 0;

        void update_request();

        game *m_game;

    public:
        request_queue(game *m_game) : m_game(m_game) {}
        
        void tick();

        void invoke_action(delayed_action &&fun);
        void queue_action(delayed_action &&fun, int priority = 0);
        void pop_request();

    public:
        size_t pending_requests() const {
            return m_requests.size();
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

        void queue_request_front(std::shared_ptr<request_base> &&value) {
            m_requests.emplace_front(std::move(value));
            update_request();
        }

        void queue_request(std::shared_ptr<request_base> &&value) {
            m_requests.emplace_back(std::move(value));
            if (m_requests.size() == 1) {
                update_request();
            }
        }

        template<std::derived_from<request_base> T>
        void queue_request_front(auto && ... args) {
            queue_request_front(std::make_shared<T>(FWD(args) ... ));
        }
        
        template<std::derived_from<request_base> T>
        void queue_request(auto && ... args) {
            queue_request(std::make_shared<T>(FWD(args) ... ));
        }
    };

}

#endif