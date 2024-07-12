#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <reflect>
#include <typeindex>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <map>

#include "event_card_key.h"

namespace banggame {

    using event_listener_fun = std::function<void(const void *tuple)>;

    template<typename T>
    auto to_event_tuple(const T &value) {
        return reflect::to<std::tuple>(value);
    }

    template<typename T>
    concept event = requires (const T &value) {
        to_event_tuple(value);
    };

    template<event T>
    using event_tuple = decltype(to_event_tuple(std::declval<const T &>()));

    template<typename Function, typename T>
    concept applicable = requires (Function fun, T tup) {
        std::apply(fun, tup);
    };

    struct event_listener {
        std::type_index type;
        event_card_key key;
        event_listener_fun fun;
        
        mutable bool active = true;

        auto operator <=> (const event_listener &other) const {
            if (type == other.type) {
                return key.priority_compare(other.key);
            } else {
                return type <=> other.type;
            }
        }

        auto operator <=> (std::type_index other_type) const {
            return type <=> other_type;
        }
    };

    class listener_map {
    public:
        using listener_set = std::multiset<event_listener, std::less<>>;
        using listener_iterator = listener_set::const_iterator;

        using iterator_map = std::multimap<event_card_key, listener_iterator, std::less<>>;
        using iterator_map_iterator = iterator_map::const_iterator;
        using iterator_map_range = rn::subrange<iterator_map_iterator>;
        
        using iterator_vector = std::vector<listener_iterator>;

    private:
        listener_set m_listeners;
        iterator_map m_map;
        iterator_vector m_to_remove;

        int m_lock = 0;

    private:
        iterator_map_iterator do_add_listener(std::type_index type, event_card_key key, event_listener_fun &&fun);
        void do_remove_listeners(iterator_map_range range);
        void do_call_event(std::type_index type, const void *tuple);

    public:
        template<event T, typename Function> requires applicable<Function, event_tuple<T>>
        iterator_map_iterator add_listener(event_card_key key, Function &&fun) {
            return do_add_listener(typeid(T), key, [fun=std::move(fun)](const void *tuple) mutable {
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
            auto tuple = to_event_tuple(value);
            do_call_event(typeid(T), &tuple);
        }
    };

}

#endif