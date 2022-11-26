#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <deque>
#include <functional>

#include "holders.h"

namespace banggame {

    using delayed_action = std::function<void()>;

    class request_queue {
    private:
        std::deque<request_holder> m_requests;
        std::deque<delayed_action> m_delayed_actions;
        int m_lock_updates = 0;

        void update_actions();
        void update_request();

        struct update_lock_guard {
            request_queue *self;
            ~update_lock_guard() noexcept(false);
        };
    
    protected:
        virtual void send_request_status_clear() = 0;
        virtual void send_request_update() = 0;

    public:
        size_t pending_requests() const {
            return m_requests.size();
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

        void pop_request() {
            m_requests.pop_front();
            send_request_status_clear();
            update_request();
        }

        [[nodiscard]] update_lock_guard lock_updates(bool pop = false);

        void tick() {
            if (pending_requests()) {
                top_request().tick();
            }
        }

        void queue_action(delayed_action &&fun);
        void queue_action_front(delayed_action &&fun);
    };

}

#endif