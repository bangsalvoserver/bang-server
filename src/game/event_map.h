#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "event_card_key.h"

#include "utils/reflector.h"

#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <map>

namespace banggame {

    struct event_listener_base {
        virtual ~event_listener_base() = default;
    };

    using event_listener_ptr = std::unique_ptr<event_listener_base>;

    template<typename T>
    concept event = reflector::reflectable<T>;

    template<event T>
    struct event_listener_interface : event_listener_base {
        static size_t get_id() {
            return reinterpret_cast<size_t>(&event_listener_interface<T>::get_id);
        }

        virtual void operator()(const reflector::as_ref_tuple_t<T> &tuple) const = 0;
    };

    template<typename Function, typename T>
    concept applicable = requires (Function fun, T tup) {
        std::apply(fun, tup);
    };

    template<typename Function, typename T>
    concept applicable_to_event = event<T> && applicable<Function, reflector::as_ref_tuple_t<T>>;

    template<typename T, typename Function> requires applicable_to_event<Function, T>
    class event_listener_impl : public event_listener_interface<T>, private Function {
    public:
        template<std::convertible_to<Function> U>
        event_listener_impl(U &&function)
            : Function(std::forward<U>(function)) {}

        void operator()(const reflector::as_ref_tuple_t<T> &tuple) const override {
            std::apply(static_cast<Function>(*this), tuple);
        }
    };

    struct event_listener {
        size_t id;
        event_listener_ptr ptr;
        event_card_key key;
        
        mutable bool active = true;

        auto operator <=> (const event_listener &other) const {
            if (id == other.id) {
                return key.priority_compare(other.key);
            } else {
                return id <=> other.id;
            }
        }

        auto operator <=> (size_t other_id) const {
            return id <=> other_id;
        }

        template<event T>
        void invoke(const void *tuple) const {
            std::invoke(static_cast<const event_listener_interface<T> &>(*ptr), *static_cast<const reflector::as_ref_tuple_t<T> *>(tuple));
        }
    };

    using event_listener_invoke_fun = void (event_listener:: *)(const void *) const;

    class listener_map {
    private:
        using listener_set = std::multiset<event_listener, std::less<>>;
        using listener_iterator = listener_set::const_iterator;

        using iterator_map = std::multimap<event_card_key, listener_iterator, std::less<>>;
        using iterator_map_range = std::ranges::subrange<iterator_map::const_iterator>;
        
        using iterator_vector = std::vector<listener_iterator>;

        listener_set m_listeners;
        iterator_map m_map;
        iterator_vector m_to_remove;

        int m_lock = 0;

    private:
        void do_add_listener(event_card_key key, size_t id, event_listener_ptr &&ptr);
        void do_remove_listeners(iterator_map_range range);
        void do_call_event(size_t id, event_listener_invoke_fun fun, const void *tuple);

    public:
        template<typename T, typename Function> requires applicable_to_event<Function, T>
        void add_listener(event_card_key key, Function &&fun) {
            do_add_listener(key, event_listener_interface<T>::get_id(),
                std::make_unique<event_listener_impl<T, std::decay_t<Function>>>(std::forward<Function>(fun)));
        }

        void remove_listeners(auto key) {
            auto [low, high] = m_map.equal_range(key);
            do_remove_listeners({low, high});
        }

        template<event T>
        void call_event(const T &value) {
            auto tuple = reflector::as_ref_tuple(value);
            do_call_event(event_listener_interface<T>::get_id(), &event_listener::invoke<T>, &tuple);
        }
    };

}

#endif