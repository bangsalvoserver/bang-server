#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <memory>
#include <concepts>
#include <functional>

#include "cards/card_effect.h"

#include "utils/stable_queue.h"

namespace banggame {

    struct request_priority_ordering {
        bool operator()(const std::shared_ptr<request_base> &lhs, const std::shared_ptr<request_base> &rhs) const {
            return lhs->priority < rhs->priority;
        }
    };

    class request_queue;

    template<std::invocable Function>
    class request_action: public request_base, private Function {
    private:
        request_queue *queue;
    
    public:
        request_action(Function &&fun, request_queue *queue, int priority)
            : request_base(nullptr, nullptr, nullptr, {}, priority)
            , Function(FWD(fun))
            , queue(queue) {}

        void on_update() override;
    };

    class request_queue {
    private:
        struct state_done {};
        struct state_next {};
        struct state_waiting { ticks timer; };
        struct state_bot_play { ticks timer; };

        using state_t = std::variant<state_done, state_next, state_waiting, state_bot_play>;

    private:
        game *m_game;

        utils::stable_priority_queue<std::shared_ptr<request_base>, request_priority_ordering> m_requests;
        state_t m_state;

        state_t invoke_update();
        state_t invoke_tick_update();

    public:
        request_queue(game *m_game) : m_game(m_game) {}
        
        void tick();
        void commit_updates();

    public:
        bool pending_requests() const {
            return !m_requests.empty();
        }

        bool is_waiting() const {
            return std::holds_alternative<state_waiting>(m_state);
        }

        template<typename T = request_base>
        std::shared_ptr<T> top_request(player *target = nullptr) {
            if (!m_requests.empty()) {
                auto req = m_requests.top();
                if (!target || req->target == target) {
                    return std::dynamic_pointer_cast<T>(req);
                }
            }
            return nullptr;
        }

        void queue_request(std::shared_ptr<request_base> &&value) {
            m_requests.emplace(std::move(value));
        }

        template<std::derived_from<request_base> T>
        void queue_request(auto && ... args) {
            queue_request(std::make_shared<T>(FWD(args) ... ));
        }

        template<std::invocable Function>
        void queue_action(Function &&fun, int priority = 0) {
            queue_request<request_action<Function>>(FWD(fun), this, priority);
        }

        void pop_request() {
            m_requests.pop();
        }
    };

    template<std::invocable Function>
    void request_action<Function>::on_update() {
        queue->pop_request();
        std::invoke(static_cast<Function &>(*this));
    }

}

#endif