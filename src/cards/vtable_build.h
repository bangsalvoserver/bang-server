#ifndef __VTABLE_BUILD_H__
#define __VTABLE_BUILD_H__

#include "game/game.h"
#include "game/filters.h"

namespace banggame {
    
    template<typename T>
    inline auto build_effect(int effect_value) {
        if constexpr (requires { T{effect_value}; }) {
            return T{effect_value};
        } else {
            return T{};
        }
    }
    
    template<typename T>
    constexpr effect_vtable build_effect_vtable(std::string_view name) {
        return {
            .name = name,

            .can_play = [](int effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> bool {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.can_play(origin_card, origin, ctx); }) {
                    return value.can_play(origin_card, origin, ctx);
                } else if constexpr (requires { value.can_play(origin_card, origin); }) {
                    return value.can_play(origin_card, origin);
                }
                return true;
            },

            .get_error = [](int effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, ctx); }) {
                    return value.get_error(origin_card, origin, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin); }) {
                    return value.get_error(origin_card, origin);
                }
                return {};
            },

            .on_prompt = [](int effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> prompt_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, ctx); }) {
                    return value.on_prompt(origin_card, origin, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin); }) {
                    return value.on_prompt(origin_card, origin);
                }
                return {};
            },

            .add_context = [](int effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, ctx); }) {
                    value.add_context(origin_card, origin, ctx);
                }
            },

            .on_play = [](int effect_value, card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_play(origin_card, origin, flags, ctx); }) {
                    value.on_play(origin_card, origin, flags, ctx);
                } else if constexpr (requires { value.on_play(origin_card, origin, ctx); }) {
                    value.on_play(origin_card, origin, ctx);
                } else if constexpr (requires { value.on_play(origin_card, origin, flags); }) {
                    value.on_play(origin_card, origin, flags);
                } else if constexpr (requires { value.on_play(origin_card, origin); }) {
                    value.on_play(origin_card, origin);
                }
            },

            .get_error_player = [](int effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, target, ctx); }) {
                    return value.get_error(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target); }) {
                    return value.get_error(origin_card, origin, target);
                }
                return {};
            },
            
            .on_prompt_player = [](int effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) -> prompt_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, target, ctx); }) {
                    return value.on_prompt(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .add_context_player = [](int effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, target, ctx); }) {
                    value.add_context(origin_card, origin, target, ctx);
                }
            },

            .on_play_player = [](int effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_play(origin_card, origin, target, flags, ctx); }) {
                    value.on_play(origin_card, origin, target, flags, ctx);
                } else if constexpr (requires { value.on_play(origin_card, origin, target, ctx); }) {
                    value.on_play(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_play(origin_card, origin, target, flags); }) {
                    value.on_play(origin_card, origin, target, flags);
                } else if constexpr (requires { value.on_play(origin_card, origin, target); }) {
                    value.on_play(origin_card, origin, target);
                }
            },

            .get_error_card = [](int effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, target, ctx); }) {
                    return value.get_error(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target); }) {
                    return value.get_error(origin_card, origin, target);
                }
                return {};
            },

            .on_prompt_card = [](int effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) -> prompt_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, target, ctx); }) {
                    return value.on_prompt(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .add_context_card = [](int effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, target, ctx); }) {
                    value.add_context(origin_card, origin, target, ctx);
                }
            },

            .on_play_card = [](int effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_play(origin_card, origin, target, flags, ctx); }) {
                    value.on_play(origin_card, origin, target, flags, ctx);
                } else if constexpr (requires { value.on_play(origin_card, origin, target, ctx); }) {
                    value.on_play(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_play(origin_card, origin, target, flags); }) {
                    value.on_play(origin_card, origin, target, flags);
                } else if constexpr (requires { value.on_play(origin_card, origin, target); }) {
                    value.on_play(origin_card, origin, target);
                }
            }
        };
    }

    #ifdef BUILD_EFFECT_VTABLE
    #undef BUILD_EFFECT_VTABLE
    #endif

    #define BUILD_EFFECT_VTABLE(name, type) template<> const effect_vtable effect_vtable_map<#name>::value = build_effect_vtable<type>(#name);
    
    template<typename T>
    constexpr equip_vtable build_equip_vtable(std::string_view name) {
        return {
            .name = name,

            .on_prompt = [](int effect_value, card_ptr origin_card, player_ptr origin, player_ptr target) -> prompt_string {
                auto value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .on_enable = [](int effect_value, card_ptr target_card, player_ptr target) {
                auto value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_enable(target_card, target); }) {
                    value.on_enable(target_card, target);
                }
            },

            .on_disable = [](int effect_value, card_ptr target_card, player_ptr target) {
                auto value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_disable(target_card, target); }) {
                    value.on_disable(target_card, target);
                }
            },

            .is_nodisable = requires { typename T::nodisable; }
        };
    }

    #ifdef BUILD_EQUIP_VTABLE
    #undef BUILD_EQUIP_VTABLE
    #endif

    #define BUILD_EQUIP_VTABLE(name, type) template<> const equip_vtable equip_vtable_map<#name>::value = build_equip_vtable<type>(#name);

    template<typename T>
    constexpr modifier_vtable build_modifier_vtable(std::string_view name) {
        return {
            .name = name,

            .add_context = [](card_ptr origin_card, player_ptr origin, effect_context &ctx) {
                T handler{};
                if constexpr (requires { handler.add_context(origin_card, origin, ctx); }) {
                    handler.add_context(origin_card, origin, ctx);
                }
            },

            .get_error = [](card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) -> game_string {
                T handler{};
                if (target_card->is_equip_card()) {
                    if constexpr (requires { handler.valid_with_equip(origin_card, origin, target_card); }) {
                        if (handler.valid_with_equip(origin_card, origin, target_card)) {
                            return {};
                        } else {
                            return {"ERROR_CANT_PLAY_WHILE_EQUIPPING", origin_card, target_card};
                        }
                    }
                } else if (target_card->get_modifier(origin->m_game->pending_requests())) {
                    if constexpr (requires { handler.valid_with_modifier(origin_card, origin, target_card); }) {
                        if (handler.valid_with_modifier(origin_card, origin, target_card)) {
                            return {};
                        } else {
                            return {"ERROR_NOT_ALLOWED_WITH_MODIFIER", origin_card, target_card};
                        }
                    }
                }
                if constexpr (requires { handler.valid_with_card(origin_card, origin, target_card); }) {
                    if (!handler.valid_with_card(origin_card, origin, target_card)) {
                        return {"ERROR_NOT_ALLOWED_WITH_CARD", origin_card, target_card};
                    }
                }
                if constexpr (requires { handler.get_error(origin_card, origin, target_card, ctx); }) {
                    return handler.get_error(origin_card, origin, target_card, ctx);
                } else if constexpr (requires { handler.get_error(origin_card, origin, target_card); }) {
                    return handler.get_error(origin_card, origin, target_card);
                }
                return {};
            }
        };
    };

    #ifdef BUILD_MODIFIER_VTABLE
    #undef BUILD_MODIFIER_VTABLE
    #endif

    #define BUILD_MODIFIER_VTABLE(name, type) template<> const modifier_vtable modifier_vtable_map<#name>::value = build_modifier_vtable<type>(#name);

    template<typename ... Ts, size_t ... Is>
    auto build_mth_args_helper(const auto &targets, std::index_sequence<Is...>) {
        return std::make_tuple(std::visit([](const auto &value) -> Ts {
            if constexpr (std::is_convertible_v<std::remove_cvref_t<decltype(value)>, Ts>) {
                return value;
            } else {
                throw game_error("invalid access to mth: wrong target type");
            }
        }, *targets[Is]) ...);
    }

    template<typename ... Ts>
    auto build_mth_args(const target_list &targets, small_int_set args) {
        static constexpr size_t N = sizeof...(Ts);

        if (args.size() != N) {
            throw game_error("invalid access to mth: invalid args size");
        }

        std::array<target_list::const_iterator, N> target_array;

        auto it = targets.begin();
        auto out_it = target_array.begin();
        int prev = 0;
        for (int arg : args) {
            for (; prev != arg && it != targets.end(); ++prev, ++it);
            if (it == targets.end()) {
                throw game_error("invalid access to mth: out of bounds");
            }
            *out_it++ = it;
        }

        return build_mth_args_helper<Ts...>(target_array, std::make_index_sequence<N>());
    }

    template<typename RetType, typename HandlerType, typename ... Args>
    using fun_mem_ptr_t = RetType (HandlerType::*)(card_ptr origin_card, player_ptr origin, Args...);

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    requires std::same_as<std::remove_cvref_t<CtxType>, effect_context>
    using ctx_fun_mem_ptr_t = RetType (HandlerType::*)(card_ptr origin_card, player_ptr origin, CtxType ctx, Args...);

    template<typename T> struct mth_unwrapper;

    template<typename RetType, typename HandlerType, typename ... Args>
    struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>> {
        fun_mem_ptr_t<RetType, HandlerType, Args...> m_value;

        RetType operator()(card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx) {
            return std::apply(m_value, std::tuple_cat(
                std::tuple{HandlerType{}, origin_card, origin},
                build_mth_args<Args...>(targets, args)
            ));
        }
    };

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    struct mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>> {
        ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...> m_value;

        RetType operator()(card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, CtxType ctx) {
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
    
    template<typename T>
    constexpr mth_vtable build_mth_vtable(std::string_view name) {
        return {
            .name = name,

            .get_error = [](card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::get_error}; }) {
                    return mth_unwrapper{&T::get_error}(origin_card, origin, targets, args, ctx);
                }
                return {};
            },

            .on_prompt = [](card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx) -> prompt_string {
                if constexpr (requires { mth_unwrapper{&T::on_prompt}; }) {
                    return mth_unwrapper{&T::on_prompt}(origin_card, origin, targets, args, ctx);
                }
                return {};
            },

            .on_play = [](card_ptr origin_card, player_ptr origin, const target_list &targets, small_int_set args, const effect_context &ctx) {
                if constexpr (requires { mth_unwrapper{&T::on_play}; }) {
                    mth_unwrapper{&T::on_play}(origin_card, origin, targets, args, ctx);
                }
            }
        };
    }

    #ifdef BUILD_MTH_VTABLE
    #undef BUILD_MTH_VTABLE
    #endif

    #define BUILD_MTH_VTABLE(name, type) template<> const mth_vtable mth_vtable_map<#name>::value = build_mth_vtable<type>(#name);
    
    template<typename T>
    constexpr ruleset_vtable build_ruleset_vtable(std::string_view name) {
        return {
            .name = name,

            .on_apply = [](game *game) {
                T value{};
                if constexpr (requires { value.on_apply(game); }) {
                    value.on_apply(game);
                }
            },

            .is_valid_with = [](const expansion_set &set) -> bool {
                T value{};
                if constexpr (requires { value.is_valid_with(set); }) {
                    return value.is_valid_with(set);
                } else {
                    return true;
                }
            },
        };
    }

    #ifdef BUILD_RULESET_VTABLE
    #undef BUILD_RULESET_VTABLE
    #endif

    #define BUILD_RULESET_VTABLE(name, type) template<> const ruleset_vtable ruleset_vtable_map<#name>::value = build_ruleset_vtable<type>(#name);
}

#endif