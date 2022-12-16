#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <deque>
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
        std::deque<request_holder> m_requests;
        utils::stable_priority_queue<action_priority_pair, action_ordering> m_delayed_actions;
        int m_lock_updates = 0;

        void update_request();

        game *m_game;

    public:
        request_queue(game *m_game) : m_game(m_game) {}
        
        void invoke_action(delayed_action &&fun);
        void queue_action(delayed_action &&fun, int priority = 0);
        void pop_request();

    public:
        size_t pending_requests() const {
            return m_requests.size();
        }

        bool locked() const {
            return pending_requests() || !m_delayed_actions.empty() || m_lock_updates;
        }

        request_holder &top_request() {
            return m_requests.front();
        }

        template<typename T = request_base>
        T *top_request_if(player *target = nullptr) {
            if (!pending_requests()) return nullptr;
            request_holder &req = top_request();
            return !target || req.target() == target ? req.get_if<T>() : nullptr;
        }

        template<typename T>
        bool top_request_is(player *target = nullptr) {
            return top_request_if<T>(target) != nullptr;
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

        void tick() {
            if (pending_requests()) {
                top_request().tick(this);
            }
        }
    };

}

#endif