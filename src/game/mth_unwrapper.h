#ifndef __MTH_UNWRAPPER_H__
#define __MTH_UNWRAPPER_H__

#include "effects/card_effect.h"

namespace banggame {

template<typename T> struct target_getter {
    T operator()(const target_list &targets, size_t index) {
        return std::get<T>(targets.at(index));
    }
};

template<target_type E> struct target_getter<tagged_value<E>> {
    tagged_value<E> operator()(const target_list &targets, size_t index) {
        if (index >= targets.size() || !targets[index].is(E)) {
            throw std::bad_variant_access();
        }
        return {};
    }
};

template<target_type E>
requires (play_card_target::has_type<E>)
struct target_getter<tagged_value<E>> {
    tagged_value<E> operator()(const target_list &targets, size_t index) {
        return {targets.at(index).get<E>()};
    }
};

template<typename T> struct target_getter<std::optional<T>> {
    std::optional<T> operator()(const target_list &targets, size_t index) {
        if (index < targets.size()) {
            return target_getter<T>{}(targets, index);
        } else {
            return std::nullopt;
        }
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
using fun_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, Args...);

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

template<typename RetType, typename HandlerType, std::integral SizeType>
struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, SizeType>> {
    fun_mem_ptr_t<RetType, HandlerType, SizeType> m_value;

    RetType operator()(card *origin_card, player *origin, const target_list &targets) {
        return (HandlerType{}.*m_value)(origin_card, origin, static_cast<SizeType>(targets.size()));
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
mth_unwrapper(fun_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>>;

}

#endif