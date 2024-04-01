#ifndef __VTABLE_MTH_H__
#define __VTABLE_MTH_H__

#include <stdexcept>

#include "utils/tstring.h"

#include "game/filters_simple.h"

namespace banggame {

    template<typename T> struct target_getter {
        T operator()(const effect_target_list &targets, size_t index) {
            if (index < targets.size()) {
                return std::visit([](const auto &value) -> T {
                    if constexpr (std::is_convertible_v<std::remove_cvref_t<decltype(value)>, T>) {
                        return value;
                    } else {
                        throw std::runtime_error("invalid access to mth: wrong target type");
                    }
                }, targets[index].target);
            } else if constexpr(std::is_pointer_v<T>) {
                return nullptr;
            } else if constexpr(std::is_default_constructible_v<std::remove_cvref_t<T>>) {
                static const std::remove_cvref_t<T> empty_value;
                return empty_value;
            } else {
                throw std::runtime_error("invalid access to mth: out of bounds");
            }
        }
    };

    template<typename T> requires std::same_as<std::remove_cvref_t<T>, effect_target_pair>
    struct target_getter<T> {
        T operator()(const effect_target_list &targets, size_t index) {
            return targets.at(index);
        }
    };

    template<> struct target_getter<bool> {
        bool operator()(const effect_target_list &targets, size_t index) {
            return index < targets.size();
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

    template<typename RetType, typename HandlerType, typename ... Args>
    mth_unwrapper(fun_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>>;

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    mth_unwrapper(ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>) -> mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>>;

    struct mth_vtable {
        std::string_view name;
        
        game_string (*get_error)(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx);
        game_string (*on_prompt)(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx);
        void (*on_play)(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx);
    };

    template<typename T>
    constexpr mth_vtable build_mth_vtable(std::string_view name) {
        return {
            .name = name,

            .get_error = [](card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::get_error}; }) {
                    return mth_unwrapper{&T::get_error}(origin_card, origin, targets, ctx);
                } else  if constexpr (requires { mth_unwrapper{&T::can_play}; }) {
                    if (!mth_unwrapper{&T::can_play}(origin_card, origin, targets, ctx)) {
                        return "ERROR_INVALID_ACTION";
                    }
                }
                return {};
            },

            .on_prompt = [](card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::on_check_target}; }) {
                    if (filters::is_player_bot(origin) && !mth_unwrapper{&T::on_check_target}(origin_card, origin, targets, ctx)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { mth_unwrapper{&T::on_prompt}; }) {
                    return mth_unwrapper{&T::on_prompt}(origin_card, origin, targets, ctx);
                }
                return {};
            },

            .on_play = [](card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx) {
                if constexpr (requires { mth_unwrapper{&T::on_play}; }) {
                    mth_unwrapper{&T::on_play}(origin_card, origin, targets, ctx);
                }
            }
        };
    }

    template<utils::tstring Name> struct mth_vtable_map;

    #define DEFINE_MTH(name, type) \
        template<> struct mth_vtable_map<#name> { \
            static constexpr mth_vtable value = build_mth_vtable<type>(#name); \
        };
    
    #define GET_MTH(name) (&mth_vtable_map<#name>::value)
}

#endif