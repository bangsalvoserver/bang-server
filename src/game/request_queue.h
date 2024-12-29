#ifndef __REQUEST_QUEUE_H__
#define __REQUEST_QUEUE_H__

#include <memory>
#include <concepts>
#include <functional>

#include "cards/card_effect.h"

#include "utils/tagged_variant.h"
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

    using request_state = utils::tagged_variant<
        utils::tag<"done">,
        utils::tag<"next">,
        utils::tag<"waiting", ticks>,
        utils::tag<"bot_play", ticks>
    >;

    using request_state_index = utils::tagged_variant_index<request_state>;

    struct any_request {
        bool operator()(const request_base &) const {
            return true;
        }
    };

    struct target_is {
        const_player_ptr target;
        target_is(const_player_ptr target): target{target} {}

        bool operator()(const request_base &req) const {
            return req.target == target;
        }
    };

    class request_queue {
    private:
        utils::stable_priority_queue<std::shared_ptr<request_base>, request_priority_ordering> m_requests;
        request_state m_state;

        request_state invoke_update();
        request_state invoke_tick_update();
    
    protected:
        virtual bool is_game_over() const = 0;
        virtual ticks get_total_update_time() const = 0;
        virtual void send_request_update() = 0;
        virtual void send_request_status_clear() = 0;
        virtual request_state send_request_status_ready() = 0;
        virtual request_state request_bot_play(bool instant) = 0;

    public:
        void tick();
        void commit_updates();

    public:
        bool pending_requests() const {
            return !m_requests.empty();
        }

        bool is_waiting() const {
            return holds_alternative<"waiting">(m_state);
        }

        template<typename T = request_base, std::predicate<const request_base &> Function = any_request>
        std::shared_ptr<T> top_request(Function &&fn = {}) {
            if (!m_requests.empty()) {
                auto req = m_requests.top();
                if (std::invoke(FWD(fn), *req)) {
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

        template<template<typename ... Ts> typename Template>
        void queue_request(auto && ... args) {
            using T = decltype(Template(FWD(args)...));
            queue_request<T>(FWD(args)...);
        }


        template<std::invocable Function>
        void queue_action(Function &&fun, int priority = 0) {
            queue_request<request_action>(FWD(fun), this, priority);
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