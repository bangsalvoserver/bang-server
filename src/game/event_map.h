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

    using event_listener_fun = std::function<void(const void *tuple)>;

    template<typename T>
    concept event = reflector::convertible_to_tuple<T>;

    template<event T>
    using event_tuple = reflector::as_tuple_t<T>;

    template<typename Function, typename T>
    concept applicable = requires (Function fun, T tup) {
        std::apply(fun, tup);
    };

    template<typename Function, typename T>
    concept applicable_to_event = event<T> && applicable<Function, event_tuple<T>>;

    struct event_listener {
        size_t id;
        event_listener_fun fun;
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
    };

    template<typename T>
    struct type_id {
        static size_t get() {
            return reinterpret_cast<size_t>(&type_id<T>::get);
        }
    };

    class listener_map {
    public:
        using listener_set = std::multiset<event_listener, std::less<>>;
        using listener_iterator = listener_set::const_iterator;

        using iterator_map = std::multimap<event_card_key, listener_iterator, std::less<>>;
        using iterator_map_iterator = iterator_map::const_iterator;
        using iterator_map_range = std::ranges::subrange<iterator_map_iterator>;
        
        using iterator_vector = std::vector<listener_iterator>;

    private:
        listener_set m_listeners;
        iterator_map m_map;
        iterator_vector m_to_remove;

        int m_lock = 0;

    private:
        iterator_map_iterator do_add_listener(event_card_key key, size_t id, event_listener_fun &&fun);
        void do_remove_listeners(iterator_map_range range);
        void do_call_event(size_t id, const void *tuple);

    public:
        template<typename T, typename Function> requires applicable_to_event<Function, T>
        iterator_map_iterator add_listener(event_card_key key, Function &&fun) {
            return do_add_listener(key, type_id<T>::get(), [fun=std::move(fun)](const void *tuple) {
                std::apply(fun, *static_cast<const event_tuple<T> *>(tuple));
            });
        }

        void remove_listener(iterator_map_iterator it) {
            do_remove_listeners({it, std::next(it)});
        }

        void remove_listeners(event_card_key key) {
            auto [low, high] = m_map.equal_range(key);
            do_remove_listeners({low, high});
        }

        void remove_listeners(card *key) {
            auto [low, high] = m_map.equal_range(key);
            do_remove_listeners({low, high});
        }

        template<event T>
        void call_event(const T &value) {
            auto tuple = reflector::as_tuple(value);
            do_call_event(type_id<T>::get(), &tuple);
        }
    };

}

#endif