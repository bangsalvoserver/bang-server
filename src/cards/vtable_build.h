#ifndef __VTABLE_BUILD_H__
#define __VTABLE_BUILD_H__

#include "game_enums.h"
#include "filter_enums.h"

#include "game/game_table.h"
#include "game/filters.h"

namespace banggame {
    
    template<typename T>
    inline T effect_cast(const void *effect_value) {
        // TODO add const to all effect member functions
        // so we can return const T &
        static_assert(std::is_trivially_copyable_v<T>);
        return *static_cast<const T *>(effect_value);
    }
    
    template<typename T>
    constexpr effect_vtable build_effect_vtable(std::string_view name) {
        return {
            .name = name,

            .can_play = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> bool {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.can_play(origin_card, origin, ctx); }) {
                    return value.can_play(origin_card, origin, ctx);
                } else if constexpr (requires { value.can_play(origin_card, origin); }) {
                    return value.can_play(origin_card, origin);
                }
                return true;
            },

            .get_error = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, ctx); }) {
                    return value.get_error(origin_card, origin, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin); }) {
                    return value.get_error(origin_card, origin);
                }
                return {};
            },

            .on_prompt = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, ctx); }) {
                    return value.on_prompt(origin_card, origin, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin); }) {
                    return value.on_prompt(origin_card, origin);
                }
                return {};
            },

            .add_context = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, ctx); }) {
                    value.add_context(origin_card, origin, ctx);
                }
            },

            .on_play = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
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

            .get_error_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, target, ctx); }) {
                    return value.get_error(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target); }) {
                    return value.get_error(origin_card, origin, target);
                }
                return {};
            },
            
            .on_prompt_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, target, ctx); }) {
                    return value.on_prompt(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .add_context_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, target, ctx); }) {
                    value.add_context(origin_card, origin, target, ctx);
                }
            },

            .on_play_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
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

            .get_error_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, target, ctx); }) {
                    return value.get_error(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target); }) {
                    return value.get_error(origin_card, origin, target);
                }
                return {};
            },

            .on_prompt_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, target, ctx); }) {
                    return value.on_prompt(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .add_context_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, target, ctx); }) {
                    value.add_context(origin_card, origin, target, ctx);
                }
            },

            .on_play_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
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
    #define BUILD_EFFECT_VALUE(name, ...) (effect_vtable_map<#name>::type{__VA_ARGS__})
    
    template<typename T>
    constexpr equip_vtable build_equip_vtable(std::string_view name) {
        return {
            .name = name,

            .on_prompt = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .on_enable = [](const void *effect_value, card_ptr target_card, player_ptr target) {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.on_enable(target_card, target); }) {
                    value.on_enable(target_card, target);
                }
            },

            .on_disable = [](const void *effect_value, card_ptr target_card, player_ptr target) {
                auto &&value = effect_cast<T>(effect_value);
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
    #define BUILD_EQUIP_VALUE(name, ...) (equip_vtable_map<#name>::type{__VA_ARGS__})

    template<typename T>
    constexpr modifier_vtable build_modifier_vtable(std::string_view name) {
        return {
            .name = name,

            .add_context = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, ctx); }) {
                    value.add_context(origin_card, origin, ctx);
                }
            },

            .get_error = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target_card, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                if (target_card->is_equip_card()) {
                    if constexpr (requires { value.valid_with_equip(origin_card, origin, target_card); }) {
                        if (value.valid_with_equip(origin_card, origin, target_card)) {
                            return {};
                        } else {
                            return {"ERROR_CANT_PLAY_WHILE_EQUIPPING", origin_card, target_card};
                        }
                    }
                } else if (target_card->get_modifier(origin->m_game->pending_requests())) {
                    if constexpr (requires { value.valid_with_modifier(origin_card, origin, target_card); }) {
                        if (value.valid_with_modifier(origin_card, origin, target_card)) {
                            return {};
                        } else {
                            return {"ERROR_NOT_ALLOWED_WITH_MODIFIER", origin_card, target_card};
                        }
                    }
                }
                if constexpr (requires { value.valid_with_card(origin_card, origin, target_card); }) {
                    if (!value.valid_with_card(origin_card, origin, target_card)) {
                        return {"ERROR_NOT_ALLOWED_WITH_CARD", origin_card, target_card};
                    }
                }
                if constexpr (requires { value.get_error(origin_card, origin, target_card, ctx); }) {
                    return value.get_error(origin_card, origin, target_card, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target_card); }) {
                    return value.get_error(origin_card, origin, target_card);
                }
                return {};
            }
        };
    };

    #ifdef BUILD_MODIFIER_VTABLE
    #undef BUILD_MODIFIER_VTABLE
    #endif

    #define BUILD_MODIFIER_VTABLE(name, type) template<> const modifier_vtable modifier_vtable_map<#name>::value = build_modifier_vtable<type>(#name);
    #define BUILD_MODIFIER_VALUE(name, ...) (modifier_vtable_map<#name>::type{__VA_ARGS__})

    template<int ... Is>
    struct indices_t {
        static constexpr std::array<int, sizeof...(Is)> value{Is ...};
    };

    template<typename T>
    struct mth_value : T {
        std::span<const int> indices;

        template<int ... Is>
        mth_value(T handler, indices_t<Is...> indices)
            : T{std::move(handler)}, indices{indices.value} {}
        
        T &handler() {
            return static_cast<T &>(*this);
        }
    };

    template<typename ... Ts>
    auto build_mth_args(const target_list &targets, std::span<const int> indices) {
        if (indices.size() != sizeof...(Ts)) {
            throw game_error("invalid access to mth: invalid indices size");
        }
        return [&]<size_t ... Is>(std::index_sequence<Is...>) {
            return std::make_tuple(std::visit([](const auto &value) -> Ts {
                using value_type = std::remove_cvref_t<decltype(value)>;
                if constexpr (reflect::size<value_type>() == 1) {
                    using inner_type = reflect::member_type<0, value_type>;
                    if constexpr (std::is_convertible_v<inner_type, Ts>) {
                        return { reflect::get<0>(value) };
                    }
                }
                throw game_error("invalid access to mth: wrong target type");
            }, targets.at(indices[Is])) ...);
        }(std::index_sequence_for<Ts ...>());
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

        RetType operator()(const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) {
            auto &&value = effect_cast<mth_value<HandlerType>>(effect_value);
            return std::apply(m_value, std::tuple_cat(
                std::tuple{value.handler(), origin_card, origin},
                build_mth_args<Args...>(targets, value.indices)
            ));
        }
    };

    template<typename RetType, typename HandlerType, typename CtxType, typename ... Args>
    struct mth_unwrapper<ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...>> {
        ctx_fun_mem_ptr_t<RetType, HandlerType, CtxType, Args...> m_value;

        RetType operator()(const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, CtxType ctx) {
            auto &&value = effect_cast<mth_value<HandlerType>>(effect_value);
            return std::apply(m_value, std::tuple_cat(
                std::tuple{value.handler(), origin_card, origin, ctx},
                build_mth_args<Args...>(targets, value.indices)
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

            .get_error = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::get_error}; }) {
                    return mth_unwrapper{&T::get_error}(effect_value, origin_card, origin, targets, ctx);
                }
                return {};
            },

            .on_prompt = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) -> prompt_string {
                if constexpr (requires { mth_unwrapper{&T::on_prompt}; }) {
                    return mth_unwrapper{&T::on_prompt}(effect_value, origin_card, origin, targets, ctx);
                }
                return {};
            },

            .on_play = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const target_list &targets, const effect_context &ctx) {
                if constexpr (requires { mth_unwrapper{&T::on_play}; }) {
                    mth_unwrapper{&T::on_play}(effect_value, origin_card, origin, targets, ctx);
                }
            }
        };
    }

    #ifdef BUILD_MTH_VTABLE
    #undef BUILD_MTH_VTABLE
    #endif

    #define BUILD_MTH_VTABLE(name, type) template<> const mth_vtable mth_vtable_map<#name>::value = build_mth_vtable<type>(#name);

    #define MAKE_INDICES(...) indices_t<__VA_ARGS__>{}
    #define BUILD_MTH_VALUE(name, indices, ...) (mth_value{ mth_vtable_map<#name>::type{__VA_ARGS__}, MAKE_INDICES indices })
    
    template<typename T>
    constexpr ruleset_vtable build_ruleset_vtable(std::string_view name) {
        return {
            .name = name,

            .on_apply = [](game_ptr game) {
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