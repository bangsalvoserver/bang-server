#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <deque>
#include <functional>

#include "holders.h"

namespace banggame {

    template<typename Derived>
    class request_queue {
    private:
        std::deque<request_holder> m_requests;
        std::deque<std::function<void()>> m_delayed_actions;

    public:
        size_t pending_requests() const {
            return m_requests.size();
        }

        size_t pending_actions() const {
            return m_delayed_actions.size();
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
        
        void update_request() {
            if (pending_requests()) {
                auto &req = top_request();
                req.on_update();
                if (!req.auto_resolve()) {
                    req.add_pending_confirms();
                    static_cast<Derived &>(*this).send_request_update();
                }
            } else {
                while (!pending_requests() && pending_actions()) {
                    auto fun = std::move(m_delayed_actions.front());
                    m_delayed_actions.pop_front();
                    std::invoke(fun);
                }
            }
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

        void pop_request() {
            m_requests.pop_front();
            static_cast<Derived &>(*this).send_request_status_clear();
        }

        void tick() {
            if (pending_requests()) {
                top_request().tick();
            }
        }

        template<std::invocable Function>
        void queue_action(Function &&fun) {
            if (!pending_requests() && !pending_actions()) {
                std::invoke(std::forward<Function>(fun));
            } else {
                m_delayed_actions.push_back(std::forward<Function>(fun));
            }
        }

        template<std::invocable Function>
        void queue_action_front(Function &&fun) {
            if (!pending_requests() && !pending_actions()) {
                std::invoke(std::forward<Function>(fun));
            } else {
                m_delayed_actions.push_front(std::forward<Function>(fun));
            }
        }
    };

}

#endif