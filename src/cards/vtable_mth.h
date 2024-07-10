#ifndef __VTABLE_MTH_H__
#define __VTABLE_MTH_H__

#include <stdexcept>

#include "card_defs.h"

#include "utils/tstring.h"

#include "game/filters_simple.h"

namespace banggame {

    template<typename ... Ts, size_t ... Is>
    auto build_mth_args_helper(const auto &targets, std::index_sequence<Is...>) {
        return std::make_tuple(std::visit([](const auto &value) -> Ts {
            if constexpr (std::is_convertible_v<std::remove_cvref_t<decltype(value)>, Ts>) {
                return value;
            } else {
                throw std::runtime_error("invalid access to mth: wrong target type");
            }
        }, *targets[Is]) ...);
    }

    template<typename ... Ts>
    auto build_mth_args(const target_list &targets, serial::int_list args) {
        static constexpr size_t N = sizeof...(Ts);

        if (args.size() != N) {
            throw std::runtime_error("invalid access to mth: invalid args size");
        }

        std::array<target_list::const_iterator, N> target_array;

        auto it = targets.begin();
        auto out_it = target_array.begin();
        int prev = 0;
        for (int arg : args) {
            for (; prev != arg && it != targets.end(); ++prev, ++it);
            if (it == targets.end()) {
                throw std::runtime_error("invalid access to mth: out of bounds");
            }
            *out_it++ = it;
        }

        return build_mth_args_helper<Ts...>(target_array, std::make_index_sequence<N>());
    }

    template<typename RetType, typename HandlerType, typename ... Args>
    using fun_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, Args...);

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    requires std::same_as<std::remove_cvref_t<CtxType>, effect_context>
    using ctx_fun_mem_ptr_t = RetType (HandlerType::*)(card *origin_card, player *origin, CtxType ctx, Args...);

    template<typename T> struct mth_unwrapper;

    template<typename RetType, typename HandlerType, typename ... Args>
    struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>> {
        fun_mem_ptr_t<RetType, HandlerType, Args...> m_value;

        RetType operator()(card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx) {
            return std::apply(m_value, std::tuple_cat(
                std::tuple{HandlerType{}, origin_card, origin},
                build_mth_args<Args...>(targets, args)
            ));
        }
    };

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    struct mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>> {
        ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...> m_value;

        RetType operator()(card *origin_card, player *origin, const target_list &targets, serial::int_list args, CtxType ctx) {
            return std::apply(m_value, std::tuple_cat(
                std::tuple{HandlerType{}, origin_card, origin, ctx},
                build_mth_args<Args...>(targets, args)
            ));
        }
    };

    template<typename RetType, typename HandlerType, typename ... Args>
    mth_unwrapper(fun_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>>;

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    mth_unwrapper(ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>) -> mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>>;

    struct mth_vtable {
        std::string_view name;
        
        game_string (*get_error)(card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx);
        game_string (*on_prompt)(card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx);
        void (*on_play)(card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx);
    };

    inline game_string mth_holder::get_error(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx) const {
        return type->get_error(origin_card, origin, targets, args, ctx);
    }

    inline game_string mth_holder::on_prompt(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx) const {
        return type->on_prompt(origin_card, origin, targets, args, ctx);
    }

    inline void mth_holder::on_play(card *origin_card, player *origin, const target_list &targets, const effect_context &ctx) const {
        type->on_play(origin_card, origin, targets, args, ctx);
    }

    template<typename T>
    constexpr mth_vtable build_mth_vtable(std::string_view name) {
        return {
            .name = name,

            .get_error = [](card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::get_error}; }) {
                    return mth_unwrapper{&T::get_error}(origin_card, origin, targets, args, ctx);
                } else  if constexpr (requires { mth_unwrapper{&T::can_play}; }) {
                    if (!mth_unwrapper{&T::can_play}(origin_card, origin, targets, args, ctx)) {
                        return "ERROR_INVALID_ACTION";
                    }
                }
                return {};
            },

            .on_prompt = [](card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::on_check_target}; }) {
                    if (filters::is_player_bot(origin) && !mth_unwrapper{&T::on_check_target}(origin_card, origin, targets, args, ctx)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { mth_unwrapper{&T::on_prompt}; }) {
                    return mth_unwrapper{&T::on_prompt}(origin_card, origin, targets, args, ctx);
                }
                return {};
            },

            .on_play = [](card *origin_card, player *origin, const target_list &targets, serial::int_list args, const effect_context &ctx) {
                if constexpr (requires { mth_unwrapper{&T::on_play}; }) {
                    mth_unwrapper{&T::on_play}(origin_card, origin, targets, args, ctx);
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