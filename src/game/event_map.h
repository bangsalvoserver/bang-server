#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "utils/priority_double_map.h"
#include "cards/event_enums.h"
#include "event_card_key.h"

namespace banggame {

    using event_map = util::priority_double_map<event_type, event_card_key, event_card_key::priority_greater>;

    template<typename T, typename U> struct same_function_args {};

    template<typename Function, typename RetType, typename ... Ts>
    struct same_function_args<Function, std::function<RetType(Ts...)>> : std::is_invocable_r<RetType, Function, Ts...> {};

    template<typename T, event_type E>
    concept invocable_for_event = same_function_args<T, enums::enum_type_t<E>>::value;

    template<typename Tuple, typename ISeqIn, typename ISeqOut>
    struct find_reference_params_impl;

    template<typename First, typename ... Ts, size_t IFirst, size_t ... Is, size_t ... Os>
    struct find_reference_params_impl<std::tuple<First, Ts...>, std::index_sequence<IFirst, Is...>, std::index_sequence<Os...>>
        : find_reference_params_impl<std::tuple<Ts...>, std::index_sequence<Is...>, std::index_sequence<Os...>> {};

    template<typename First, typename ... Ts, size_t IFirst, size_t ... Is, size_t ... Os> requires (!std::is_const_v<First>)
    struct find_reference_params_impl<std::tuple<First&, Ts...>, std::index_sequence<IFirst, Is...>, std::index_sequence<Os...>>
        : find_reference_params_impl<std::tuple<Ts...>, std::index_sequence<Is...>, std::index_sequence<Os..., IFirst>> {};

    template<size_t ... Os>
    struct find_reference_params_impl<std::tuple<>, std::index_sequence<>, std::index_sequence<Os...>> {
        using type = std::index_sequence<Os...>;
    };

    template<typename ... Ts>
    struct find_reference_params : find_reference_params_impl<std::tuple<Ts...>, std::index_sequence_for<Ts...>, std::index_sequence<>> {};

    template<typename ... Ts>
    using find_reference_params_t = typename find_reference_params<Ts...>::type;

    template<typename Function>
    struct filter_reference_params_impl;

    template<typename RetType, typename ... Ts>
    struct filter_reference_params_impl<std::function<RetType(Ts...)>> {
        template<typename Tuple, size_t ... Is>
        auto operator ()(const Tuple &tup, std::index_sequence<Is ...>) const {
            return std::make_tuple(std::get<Is>(tup) ... );
        }

        template<typename Tuple>
        auto operator ()(const Tuple &tup) const {
            return (*this)(tup, find_reference_params_t<Ts...>{});
        }
    };

    template<typename Function, typename Tuple>
    auto filter_reference_params(const Tuple &tup) {
        return filter_reference_params_impl<Function>{}(tup);
    }

    template<typename Function> struct function_argument_tuple;
    template<typename Function> using function_argument_tuple_t = typename function_argument_tuple<Function>::type;

    template<typename RetType, typename ... Ts>
    struct function_argument_tuple<std::function<RetType(Ts...)>> {
        using type = std::tuple<std::remove_reference_t<Ts> ...>;
    };

    class listener_map {
    private:
        event_map m_listeners;

    public:
        template<event_type E, invocable_for_event<E> Function>
        void add_listener(event_card_key key, Function &&fun) {
            m_listeners.add<E>(key, std::forward<Function>(fun));
            m_listeners.commit_changes();
        }

        void remove_listeners(auto key) {
            m_listeners.erase(key);
            m_listeners.commit_changes();
        }

        template<event_type E, typename ... Ts>
        auto call_event(Ts && ... args) {
            using function_type = enums::enum_type_t<E>;
            function_argument_tuple_t<function_type> tup{FWD(args) ...};

            for (auto &fun : m_listeners.get_table<E>()) {
                std::apply(fun, tup);
            }
            m_listeners.commit_changes();
            
            auto ret = filter_reference_params<function_type>(tup);
            if constexpr (std::tuple_size_v<decltype(ret)> == 1) {
                return std::get<0>(ret);
            } else if constexpr (std::tuple_size_v<decltype(ret)> > 1) {
                return ret;
            }
        }
    };

}

#endif