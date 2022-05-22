#ifndef __MTH_UNWRAPPER_H__
#define __MTH_UNWRAPPER_H__

#include "effects/card_effect.h"

namespace banggame {

template<typename T> T get_target(const play_card_target &target) {
    return std::get<T>(target);
}
template<> player *get_target(const play_card_target &target) {
    return std::get<target_player_t>(target).target;
}
template<> card *get_target(const play_card_target &target) {
    return std::get<target_card_t>(target).target;
}

template<typename T> struct target_getter {
    T operator()(const target_list &targets, size_t index) {
        return get_target<T>(targets.at(index));
    }
};

template<typename T> struct target_getter<std::optional<T>> {
    std::optional<T> operator()(const target_list &targets, size_t index) {
        if (index < targets.size()) {
            return get_target<T>(targets[index]);
        } else {
            return std::nullopt;
        }
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
using fun_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, Args...);

template<typename RetType, typename HandlerType, typename ... Args>
using const_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, Args...) const;

template<typename T> struct mth_unwrapper;

template<typename RetType, typename HandlerType, typename ... Args>
struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>> {
    fun_mem_ptr_t<RetType, HandlerType, Args...> m_value;

    RetType operator()(card *origin_card, player *origin, const target_list &targets) {
        return [&]<size_t ... Is>(std::index_sequence<Is...>) {
            return (HandlerType{}.*m_value)(origin_card, origin, target_getter<Args>{}(targets, Is) ...);
        }(std::index_sequence_for<Args...>());
    }
};

template<typename RetType, typename HandlerType>
struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, const target_list &>> {
    fun_mem_ptr_t<RetType, HandlerType, const target_list &> m_value;

    RetType operator()(card *origin_card, player *origin, const target_list &targets) {
        return (HandlerType{}.*m_value)(origin_card, origin, targets);
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
struct mth_unwrapper<const_mem_ptr_t<RetType, HandlerType, Args...>> {
    const_mem_ptr_t<RetType, HandlerType, Args...> m_value;

    RetType operator()(card *origin_card, player *origin, const target_list &targets) const {
        return [&]<size_t ... Is>(std::index_sequence<Is...>) {
            return (HandlerType{}.*m_value)(origin_card, origin, target_getter<Args>{}(targets, Is) ...);
        }(std::index_sequence_for<Args...>());
    }
};

template<typename RetType, typename HandlerType>
struct mth_unwrapper<const_mem_ptr_t<RetType, HandlerType, const target_list &>> {
    const_mem_ptr_t<RetType, HandlerType, const target_list &> m_value;

    RetType operator()(card *origin_card, player *origin, const target_list &targets) const {
        return (HandlerType{}.*m_value)(origin_card, origin, targets);
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
mth_unwrapper(fun_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>>;

template<typename RetType, typename HandlerType, typename ... Args>
mth_unwrapper(const_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<const_mem_ptr_t<RetType, HandlerType, Args...>>;

}

#endif