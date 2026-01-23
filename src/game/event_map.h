#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <reflect>
#include <typeindex>
#include <functional>
#include <memory>
#include <vector>
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

    template<typename T>
    struct event_result_impl { using type = void; };

    template<typename T> requires requires { typename T::result_type; }
    struct event_result_impl<T> { using type = typename T::result_type; };

    template<typename T>
    using event_result = typename event_result_impl<T>::type;

    template<typename Function, typename T>
    concept applicable_as_event = requires (Function fun, event_tuple<T> tup) {
        { std::apply(fun, tup) } -> std::convertible_to<event_result<T>>;
    };

    struct event_listener_key {
        std::type_index type;
        event_card_key key;

        auto operator <=> (const event_listener_key &other) const {
            if (type != other.type) {
                return type <=> other.type;
            } else if (key.priority != other.key.priority) {
                return other.key.priority <=> key.priority;
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
        std::move_only_function<bool(const void *tuple, void *result)> m_fun;
        std::type_index m_type;
        bool m_active = true;
    
    public:
        template<event T, typename Function> requires applicable_as_event<Function, T>
        event_listener(std::in_place_type_t<T>, Function &&fun)
            : m_fun{[fun=std::move(fun)](const void *tuple, void *result) mutable {
                using tuple_type = event_tuple<T>;
                using result_type = event_result<T>;
                const tuple_type &tuple_ref = *static_cast<const tuple_type *>(tuple);
                if constexpr (std::is_void_v<result_type>) {
                    std::apply(fun, tuple_ref);
                } else if (result_type value = std::apply(fun, tuple_ref); static_cast<bool>(value)) {
                    *static_cast<result_type *>(result) = std::move(value);
                    return true;
                }
                return false;
            }},
            m_type{typeid(Function)} {}
        
        bool operator()(const void *tuple, void *result) {
            return m_fun(tuple, result);
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
        void do_call_event(std::type_index type, const void *tuple, void *result);

    public:
        template<event T, typename Function> requires applicable_as_event<Function, T>
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
        auto call_event(const T &value) {
            auto tuple = to_event_tuple(value);
            using result_type = event_result<T>;
            if constexpr (std::is_void_v<result_type>) {
                do_call_event(typeid(T), &tuple, nullptr);
            } else {
                result_type result{};
                do_call_event(typeid(T), &tuple, &result);
                return result;
            }
        }
    };

}

#endif