#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "event_card_key.h"

#include "utils/reflector.h"

#include <functional>
#include <memory>
#include <unordered_set>
#include <set>
#include <map>

namespace banggame::detail {

    struct event_listener_base {
        const size_t id;
        const int priority;
        bool active = true;

        event_listener_base(size_t id, int priority)
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

        template<typename T>
        void invoke(const void *tuple) const;
    };

    using event_listener_invoke_fun = void (event_listener_base:: *)(const void *) const;

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
        static size_t get_id() {
            return reinterpret_cast<size_t>(&event_listener<T>::get_id);
        }

        event_listener(size_t priority): event_listener_base(get_id(), priority) {}

        virtual void operator()(const build_tuple_t<T> &tuple) const = 0;
    };

    template<typename T>
    void event_listener_base::invoke(const void *tuple) const {
        std::invoke(static_cast<const event_listener<T> &>(*this), *static_cast<const build_tuple_t<T> *>(tuple));
    }

    template<typename Function, typename Tuple>
    concept applicable = requires (Function fun, Tuple tup) {
        std::apply(fun, tup);
    };

    template<typename T, typename Function> requires applicable<Function, build_tuple_t<T>>
    class event_listener_impl : public event_listener<T>, private Function {
    public:
        template<std::convertible_to<Function> U>
        event_listener_impl(size_t priority, U &&function)
            : event_listener<T>(priority)
            , Function(std::forward<U>(function)) {}

        void operator()(const build_tuple_t<T> &tuple) const override {
            std::apply(static_cast<Function>(*this), tuple);
        }
    };

    template<typename T>
    concept dereferenceable = requires(T ptr) {
        *ptr;
    };

    template<dereferenceable T> using deref_t = std::decay_t<decltype(*std::declval<const T &>())>;

    template<typename T, typename U>
    concept comparable = requires (T lhs, U rhs) {
        lhs < rhs;
    };

    struct deref_less {
        struct is_transparent {};

        template<dereferenceable T>
        bool operator()(const T &lhs, const T &rhs) const {
            return *lhs < *rhs;
        }

        template<dereferenceable T, typename U> requires comparable<deref_t<T>, U>
        bool operator()(const T &lhs, const U &rhs) const {
            return *lhs < rhs;
        }

        template<dereferenceable T, typename U> requires comparable<deref_t<T>, U>
        bool operator()(const U &lhs, const T &rhs) const {
            return lhs < *rhs;
        }
    };

}

namespace banggame {

    template<typename T>
    constexpr size_t event_listener_buffer_size = 16;

    class listener_map {
    private:
        using listener_ptr = std::unique_ptr<detail::event_listener_base>;
        using listener_set = std::multiset<listener_ptr, detail::deref_less>;
        using listener_iterator = listener_set::const_iterator;

        using iterator_map = std::multimap<event_card_key, listener_iterator, std::less<>>;
        using iterator_map_range = std::ranges::subrange<iterator_map::const_iterator>;
        
        using iterator_vector = std::vector<listener_iterator>;

        listener_set m_listeners;
        iterator_map m_map;
        iterator_vector m_to_remove;

        int m_lock = 0;

    private:
        void do_add_listener(event_card_key key, listener_ptr &&ptr);
        void do_remove_listeners(iterator_map_range range);
        void do_call_event(size_t id, detail::event_listener_invoke_fun fun, const void *tuple);

    public:
        template<typename T, typename Function> requires detail::applicable<Function, detail::build_tuple_t<T>>
        void add_listener(event_card_key key, Function &&fun) {
            do_add_listener(key, std::make_unique<detail::event_listener_impl<T, std::decay_t<Function>>>(key.priority, std::forward<Function>(fun)));
        }

        void remove_listeners(auto key) {
            auto [low, high] = m_map.equal_range(key);
            do_remove_listeners({low, high});
        }

        template<typename T>
        void call_event(const T &value) {
            auto tuple = detail::build_tuple(value);
            do_call_event(detail::event_listener<T>::get_id(), &detail::event_listener_base::invoke<T>, &tuple);
        }
    };

}

#endif