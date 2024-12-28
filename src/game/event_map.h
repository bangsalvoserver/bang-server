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

    struct event_listener_key {
        std::type_index type;
        event_card_key key;

        auto operator <=> (const event_listener_key &other) const {
            if (type != other.type) {
                return type <=> other.type;
            } else if (key.priority != other.key.priority) {
                return key.priority <=> other.key.priority;
            } else {
                return get_card_order(key.target_card) <=> get_card_order(other.key.target_card);
            }
        }

        auto operator <=> (std::type_index other_type) const {
            return type <=> other_type;
        }
    };

    class event_listener {
    private:
        std::move_only_function<void(const void *tuple)> m_fun;
        std::type_index m_type;
        bool m_active = true;
    
    public:
        template<event T, typename Function> requires applicable<Function, event_tuple<T>>
        event_listener(std::in_place_type_t<T>, Function &&fun)
            : m_fun{[fun=std::move(fun)](const void *tuple) mutable {
                std::apply(fun, *static_cast<const event_tuple<T> *>(tuple));
            }},
            m_type{typeid(Function)} {}
        
        void operator()(const void *tuple) {
            m_fun(tuple);
        }

        const std::type_index &target_type() const {
            return m_type;
        }

        bool is_active() const {
            return m_active;
        }

        void deactivate() {
            m_active = false;
        }
    };

    class listener_map {
    private:
        using listener_set = std::multimap<event_listener_key, event_listener, std::less<>>;
        using listener_iterator = listener_set::iterator;

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
        void do_add_listener(event_listener_key key, event_listener &&listener);
        void do_remove_listeners(iterator_map_range range);
        void do_call_event(std::type_index type, const void *tuple);

    public:
        template<event T, typename Function> requires applicable<Function, event_tuple<T>>
        void add_listener(event_card_key key, Function &&fun) {
            do_add_listener({ typeid(T), key }, { std::in_place_type<T>, std::forward<Function>(fun) });
        }

        void remove_listeners(event_card_key key) {
            auto [low, high] = m_map.equal_range(key);
            do_remove_listeners({low, high});
        }

        void remove_listeners(card_ptr key) {
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