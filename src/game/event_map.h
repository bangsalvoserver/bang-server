#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "event_card_key.h"

#include "utils/reflector.h"

#include <functional>
#include <memory>
#include <set>
#include <map>

namespace banggame::detail {
    struct event_listener_base {
        const size_t id;
        const size_t priority;

        event_listener_base(size_t id, size_t priority)
            : id{id}
            , priority{priority} {}

        virtual ~event_listener_base() = default;

        auto operator <=> (const event_listener_base &other) const {
            if (id == other.id) {
                return other.priority <=> priority;
            } else {
                return id <=> other.id;
            }
        }

        auto operator <=> (size_t other_id) const {
            return id <=> other_id;
        }
    };

    struct global_unique_id {
        static inline size_t count = 0;
        const size_t value;

        global_unique_id() : value(++count) {}
    };

    template<reflector::reflectable T, size_t ... Is>
    auto build_tuple_helper(const T &value, std::index_sequence<Is...>) {
        return std::tie(reflector::get_field_data<Is, T>(value).get() ...);
    }

    template<reflector::reflectable T>
    auto build_tuple(const T &value) {
        return build_tuple_helper(value, std::make_index_sequence<reflector::num_fields<T>>());
    }

    template<typename T>
    using build_tuple_t = decltype(build_tuple(std::declval<const T &>()));

    template<typename T>
    struct event_listener : event_listener_base {
        static inline const global_unique_id id;

        event_listener(size_t priority): event_listener_base(id.value, priority) {}

        virtual void operator()(const T &value) = 0;
    };

    template<typename Function, typename Tuple>
    concept appliable = requires (Function fun, Tuple tup) {
        std::apply(fun, tup);
    };

    template<typename T, typename Function> requires appliable<Function, build_tuple_t<T>>
    class event_listener_impl : public event_listener<T>, private std::decay_t<Function> {
    private:
        using base = std::decay_t<Function>;

    public:
        template<std::convertible_to<Function> U>
        event_listener_impl(size_t priority, U &&function)
            : event_listener<T>(priority)
            , base(std::forward<U>(function)) {}

        void operator()(const T &value) override {
            std::apply(static_cast<Function &>(*this), build_tuple(value));
        }
    };

    struct shared_ptr_less {
        struct is_transparent {};

        template<typename T>
        bool operator()(const std::shared_ptr<T> &lhs, const std::shared_ptr<T> &rhs) const {
            return *lhs < *rhs;
        }

        template<typename T, typename U>
        bool operator()(const std::shared_ptr<T> &lhs, const U &rhs) const {
            return *lhs < rhs;
        }

        template<typename T, typename U>
        bool operator()(const U &lhs, const std::shared_ptr<T> &rhs) const {
            return lhs < *rhs;
        }
    };

}

namespace banggame {

    class listener_map {
    private:
        using listener_set = std::multiset<std::shared_ptr<detail::event_listener_base>, detail::shared_ptr_less>;
        using iterator_map = std::multimap<event_card_key, listener_set::const_iterator, std::less<>>;

        listener_set m_listeners;
        iterator_map m_map;

    public:
        template<typename T, typename Function> requires detail::appliable<Function, detail::build_tuple_t<T>>
        void add_listener(event_card_key key, Function &&fun) {
            auto it = m_listeners.emplace(std::make_shared<detail::event_listener_impl<T, Function>>(key.priority, std::forward<Function>(fun)));
            m_map.emplace(key, it);
        }

        void remove_listeners(auto key) {
            auto [low, high] = m_map.equal_range(key);
            if (low != high) {
                for (auto it = low; it != high; ++it) {
                    m_listeners.erase(it->second);
                }
                m_map.erase(low, high);
            }
        }

        template<typename T, typename ... Ts>
        void call_event(Ts && ... args) {
            using listener_type = detail::event_listener<T>;
            auto [low, high] = m_listeners.equal_range(listener_type::id.value);
            if (low != high) {
                auto listeners = ranges::subrange(low, high)
                    | ranges::views::transform([](const std::shared_ptr<detail::event_listener_base> &ptr) {
                        return std::static_pointer_cast<listener_type>(ptr);
                    })
                    | ranges::to<std::vector>;
                
                T value{FWD(args) ...};
                for (const std::shared_ptr<listener_type> &ptr : listeners) {
                    std::invoke(*ptr, value);
                }
            }
        }
    };

}

#endif