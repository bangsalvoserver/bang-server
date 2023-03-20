#ifndef __MTH_UNWRAPPER_H__
#define __MTH_UNWRAPPER_H__

#include "cards/card_effect.h"

namespace banggame {

template<typename T> struct target_getter {
    T operator()(const effect_target_list &targets, size_t index) {
        return std::get<T>(targets.at(index).target);
    }
};

template<> struct target_getter<card *> {
    card *operator()(const effect_target_list &targets, size_t index) {
        return targets.at(index).target.get<target_type::card>();
    }
};

template<> struct target_getter<player *> {
    player *operator()(const effect_target_list &targets, size_t index) {
        return targets.at(index).target.get<target_type::player>();
    }
};

template<typename T> requires std::same_as<std::remove_cvref_t<T>, effect_target_pair>
struct target_getter<T> {
    T operator()(const effect_target_list &targets, size_t index) {
        return targets.at(index);
    }
};

template<target_type E> struct target_getter<tagged_value<E>> {
    tagged_value<E> operator()(const effect_target_list &targets, size_t index) {
        if (index >= targets.size() || !targets[index].target.is(E)) {
            throw std::bad_variant_access();
        }
        return {};
    }
};

template<target_type E>
requires (play_card_target::has_type<E>)
struct target_getter<tagged_value<E>> {
    tagged_value<E> operator()(const effect_target_list &targets, size_t index) {
        return {targets.at(index).target.get<E>()};
    }
};

template<typename T> struct target_getter<std::optional<T>> {
    std::optional<T> operator()(const effect_target_list &targets, size_t index) {
        if (index < targets.size()) {
            return target_getter<T>{}(targets, index);
        } else {
            return std::nullopt;
        }
    }
};

template<> struct target_getter<bool> {
    bool operator()(const effect_target_list &targets, size_t index) {
        return target_getter<opt_tagged_value<target_type::none>>{}(targets, index).has_value();
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
using fun_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, Args...);

template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
requires std::same_as<std::remove_cvref_t<CtxType>, effect_context>
using ctx_fun_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, CtxType ctx, Args...);

template<typename T> struct mth_unwrapper;

template<typename RetType, typename HandlerType, typename ... Args>
struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>> {
    fun_mem_ptr_t<RetType, HandlerType, Args...> m_value;

    RetType operator()(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx) {
        return [&]<size_t ... Is>(std::index_sequence<Is...>) {
            return (HandlerType{}.*m_value)(origin_card, origin, target_getter<Args>{}(targets, Is) ...);
        }(std::index_sequence_for<Args...>());
    }
};

template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
struct mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>> {
    ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...> m_value;

    RetType operator()(card *origin_card, player *origin, const effect_target_list &targets, CtxType ctx) {
        return [&]<size_t ... Is>(std::index_sequence<Is...>) {
            return (HandlerType{}.*m_value)(origin_card, origin, ctx, target_getter<Args>{}(targets, Is) ...);
        }(std::index_sequence_for<Args...>());
    }
};

template<typename RetType, typename HandlerType>
struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, const effect_target_list &>> {
    fun_mem_ptr_t<RetType, HandlerType, const effect_target_list &> m_value;

    RetType operator()(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx) {
        return (HandlerType{}.*m_value)(origin_card, origin, targets);
    }
};

template<typename RetType, typename HandlerType, typename CtxType>
struct mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, const effect_target_list &>> {
    ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, const effect_target_list &> m_value;

    RetType operator()(card *origin_card, player *origin, const effect_target_list &targets, CtxType ctx) {
        return (HandlerType{}.*m_value)(origin_card, origin, ctx, targets);
    }
};

template<typename RetType, typename HandlerType, std::integral SizeType>
struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, SizeType>> {
    fun_mem_ptr_t<RetType, HandlerType, SizeType> m_value;

    RetType operator()(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx) {
        return (HandlerType{}.*m_value)(origin_card, origin, static_cast<SizeType>(targets.size()));
    }
};

template<typename RetType, typename HandlerType, typename CtxType, std::integral SizeType>
struct mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, SizeType>> {
    ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, SizeType> m_value;

    RetType operator()(card *origin_card, player *origin, const effect_target_list &targets, CtxType ctx) {
        return (HandlerType{}.*m_value)(origin_card, origin, ctx, static_cast<SizeType>(targets.size()));
    }
};

template<typename RetType, typename HandlerType, typename ... Args>
mth_unwrapper(fun_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>>;

template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
mth_unwrapper(ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>) -> mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>>;

}

#endif