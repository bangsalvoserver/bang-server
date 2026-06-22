#ifndef __VTABLE_BUILD_H__
#define __VTABLE_BUILD_H__

#include "game_enums.h"
#include "filter_enums.h"

#include "card_serial.h"

#include "game/game_table.h"
#include "game/filters.h"

#include "utils/random_element.h"

namespace banggame {

    #define TRY_RETURN(...) if constexpr (requires { __VA_ARGS__; }) return __VA_ARGS__
    
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
                TRY_RETURN(value.can_play(origin_card, origin, ctx));
                TRY_RETURN(value.can_play(origin_card, origin));
                return true;
            },

            .get_error = [](const void *effect_value, card_ptr origin_card, player_ptr origin, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.get_error(origin_card, origin, ctx));
                TRY_RETURN(value.get_error(origin_card, origin));
                return {};
            },

            .on_prompt = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_prompt(origin_card, origin, flags, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, flags));
                TRY_RETURN(value.on_prompt(origin_card, origin));
                return {};
            },

            .add_context = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.add_context(origin_card, origin, ctx));
            },

            .on_play = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_flags flags, const effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_play(origin_card, origin, flags, ctx));
                TRY_RETURN(value.on_play(origin_card, origin, ctx));
                TRY_RETURN(value.on_play(origin_card, origin, flags));
                TRY_RETURN(value.on_play(origin_card, origin));
            },

            .get_error_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.get_error(origin_card, origin, target, ctx));
                TRY_RETURN(value.get_error(origin_card, origin, target));
                return {};
            },
            
            .on_prompt_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_prompt(origin_card, origin, target, flags, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, target, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, target, flags));
                TRY_RETURN(value.on_prompt(origin_card, origin, target));
                return {};
            },

            .add_context_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.add_context(origin_card, origin, target, ctx));
            },

            .on_play_player = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_play(origin_card, origin, target, flags, ctx));
                TRY_RETURN(value.on_play(origin_card, origin, target, ctx));
                TRY_RETURN(value.on_play(origin_card, origin, target, flags));
                TRY_RETURN(value.on_play(origin_card, origin, target));
            },

            .get_error_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, const effect_context &ctx) -> game_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.get_error(origin_card, origin, target, ctx));
                TRY_RETURN(value.get_error(origin_card, origin, target));
                return {};
            },

            .on_prompt_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_prompt(origin_card, origin, target, flags, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, target, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, target, flags));
                TRY_RETURN(value.on_prompt(origin_card, origin, target));
                return {};
            },

            .add_context_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.add_context(origin_card, origin, target, ctx));
            },

            .on_play_card = [](const void *effect_value, card_ptr origin_card, player_ptr origin, card_ptr target, effect_flags flags, const effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_play(origin_card, origin, target, flags, ctx));
                TRY_RETURN(value.on_play(origin_card, origin, target, ctx));
                TRY_RETURN(value.on_play(origin_card, origin, target, flags));
                TRY_RETURN(value.on_play(origin_card, origin, target));
            }
        };
    }

    #ifdef BUILD_EFFECT_VTABLE
    #undef BUILD_EFFECT_VTABLE
    #endif

    #define BUILD_EFFECT_VTABLE(name, type) template<> const effect_vtable effect_vtable_map<#name>::value = build_effect_vtable<type>(#name);
    #define EFFECT_VALUE(name) effect_vtable_map<#name>::type
    
    template<typename T>
    constexpr equip_vtable build_equip_vtable(std::string_view name) {
        return {
            .name = name,

            .on_prompt = [](const void *effect_value, card_ptr origin_card, player_ptr origin, player_ptr target, const effect_context &ctx) -> prompt_string {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_prompt(origin_card, origin, target, ctx));
                TRY_RETURN(value.on_prompt(origin_card, origin, target));
                return {};
            },

            .on_enable = [](const void *effect_value, card_ptr target_card, player_ptr target) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_enable(target_card, target));
            },

            .on_disable = [](const void *effect_value, card_ptr target_card, player_ptr target) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.on_disable(target_card, target));
            },

            .is_nodisable = requires { typename T::nodisable; }
        };
    }

    #ifdef BUILD_EQUIP_VTABLE
    #undef BUILD_EQUIP_VTABLE
    #endif

    #define BUILD_EQUIP_VTABLE(name, type) template<> const equip_vtable equip_vtable_map<#name>::value = build_equip_vtable<type>(#name);
    #define EQUIP_VALUE(name) equip_vtable_map<#name>::type

    template<typename T>
    constexpr modifier_vtable build_modifier_vtable(std::string_view name) {
        return {
            .name = name,

            .add_context = [](const void *effect_value, card_ptr origin_card, player_ptr origin, effect_context &ctx) {
                auto &&value = effect_cast<T>(effect_value);
                TRY_RETURN(value.add_context(origin_card, origin, ctx));
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
                } else if (target_card->get_modifier(origin->m_game->pending_requests() ? effect_list_type::responses : effect_list_type::effects)) {
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
                TRY_RETURN(value.get_error(origin_card, origin, target_card, ctx));
                TRY_RETURN(value.get_error(origin_card, origin, target_card));
                return {};
            }
        };
    };

    #ifdef BUILD_MODIFIER_VTABLE
    #undef BUILD_MODIFIER_VTABLE
    #endif

    #define BUILD_MODIFIER_VTABLE(name, type) template<> const modifier_vtable modifier_vtable_map<#name>::value = build_modifier_vtable<type>(#name);
    #define MODIFIER_VALUE(name) modifier_vtable_map<#name>::type

    using mth_index = uint8_t;
    using index_list = std::span<const mth_index>;

    template<typename HandlerType>
    struct mth_value {
        [[no_unique_address]] HandlerType handler;
        index_list indices;
    };

    template<typename T>
    T get_target_value(targets_view targets, mth_index index) {
        try {
            if (index < targets.size()) {
                return targets[index].get<T>();
            } else {
                throw game_error("invalid access to mth: index out of range");
            }
        } catch (const std::bad_any_cast &) {
            throw game_error("invalid access to mth: invalid target type");
        }
    }

    template<typename ... Ts>
    std::tuple<Ts ...> build_mth_args(targets_view targets, index_list indices) {
        if (indices.size() != sizeof...(Ts)) {
            throw game_error("invalid access to mth: invalid indices size");
        }
        return [&]<size_t ... Is>(std::index_sequence<Is...>) {
            return std::tuple<Ts ...>{
                get_target_value<Ts>(targets, indices[Is]) ...
            };
        }(std::index_sequence_for<Ts ...>());
    }

    template<typename ... Ts>
    std::tuple<Ts ...> build_mth_args(targets_view targets, index_list indices, const effect_context &ctx) {
        using args_tuple = std::tuple<Ts ...>;
        if constexpr (sizeof...(Ts) == 0) {
            return build_mth_args<>(targets, indices);
        } else if constexpr (std::is_convertible_v<const effect_context &, std::tuple_element_t<sizeof...(Ts) - 1, args_tuple>>) {
            return std::tuple_cat(
                [&]<size_t ... Is>(std::index_sequence<Is ...>) {
                    return build_mth_args<std::tuple_element_t<Is, args_tuple> ...>(targets, indices);
                }(std::make_index_sequence<sizeof...(Ts) - 1>()),
                std::tie(ctx)
            );
        } else {
            return build_mth_args<Ts ...>(targets, indices);
        }
    }

    template<typename RetType, typename HandlerType, typename ... Args>
    using fun_mem_ptr_t = RetType (HandlerType::*)(card_ptr origin_card, player_ptr origin, Args...);

    template<typename T> struct mth_unwrapper;

    template<typename RetType, typename HandlerType, typename ... Args>
    struct mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>> {
        fun_mem_ptr_t<RetType, HandlerType, Args...> m_value;

        RetType operator()(const void *effect_value, card_ptr origin_card, player_ptr origin, targets_view targets, const effect_context &ctx) {
            auto &&value = effect_cast<mth_value<HandlerType>>(effect_value);
            return std::apply(m_value, std::tuple_cat(
                std::tie(value.handler, origin_card, origin),
                build_mth_args<Args...>(targets, value.indices, ctx)
            ));
        }
    };

    template<typename RetType, typename HandlerType, typename ... Args>
    mth_unwrapper(fun_mem_ptr_t<RetType, HandlerType, Args...>) -> mth_unwrapper<fun_mem_ptr_t<RetType, HandlerType, Args...>>;
    
    template<typename T>
    constexpr mth_vtable build_mth_vtable(std::string_view name) {
        return {
            .name = name,

            .get_error = [](const void *effect_value, card_ptr origin_card, player_ptr origin, targets_view targets, const effect_context &ctx) -> game_string {
                if constexpr (requires { mth_unwrapper{&T::get_error}; }) {
                    return mth_unwrapper{&T::get_error}(effect_value, origin_card, origin, targets, ctx);
                }
                return {};
            },

            .on_prompt = [](const void *effect_value, card_ptr origin_card, player_ptr origin, targets_view targets, const effect_context &ctx) -> prompt_string {
                if constexpr (requires { mth_unwrapper{&T::on_prompt}; }) {
                    return mth_unwrapper{&T::on_prompt}(effect_value, origin_card, origin, targets, ctx);
                }
                return {};
            },

            .on_play = [](const void *effect_value, card_ptr origin_card, player_ptr origin, targets_view targets, const effect_context &ctx) {
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

    #define MTH_VALUE(name) mth_value<mth_vtable_map<#name>::type>
    
    template<typename T>
    constexpr ruleset_vtable build_ruleset_vtable(std::string_view name) {
        return {
            .name = name,

            .on_apply = [](game_ptr game) {
                TRY_RETURN(T{}.on_apply(game));
            },

            .is_valid_with = [](const expansion_set &set) -> bool {
                TRY_RETURN(T{}.is_valid_with(set));
                return true;
            },
        };
    }

    #ifdef BUILD_RULESET_VTABLE
    #undef BUILD_RULESET_VTABLE
    #endif

    #define BUILD_RULESET_VTABLE(name, type) template<> const ruleset_vtable ruleset_vtable_map<#name>::value = build_ruleset_vtable<type>(#name);

    template<typename T>
    constexpr targeting_vtable build_targeting_vtable(std::string_view name) {
        using value_type = typename T::value_type;

        return {
            .name = name,
            
            .deserialize_target = [](const json::json &value, const game_context &context) -> play_card_target {
                return play_card_target{json::deserialize<value_type, game_context>(value, context)};
            },

            .serialize_args = [](const effect_holder &effect, json::string_writer &writer) {
                auto &&handler = effect_cast<T>(effect.target_value);
                if constexpr (requires { handler.get_args(); }) {
                    using args_t = std::remove_reference_t<decltype(handler.get_args())>;
                    using serializer_type = json::aggregate_serializer<args_t, json::no_context>;
                    serializer_type::write_fields(handler.get_args(), writer, {});
                }
            },

            .possible_targets = [](card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) -> std::generator<play_card_target> {
                auto &&handler = effect_cast<T>(effect.target_value);
                if constexpr (requires { handler.is_possible(origin_card, origin, effect, ctx); }) {
                    if (handler.is_possible(origin_card, origin, effect, ctx)) {
                        co_yield play_card_target{value_type{}};
                    }
                } else {
                    for (value_type value : handler.possible_targets(origin_card, origin, effect, ctx)) {
                        co_yield play_card_target{std::move(value)};
                    }
                }
            },

            .random_target = [](card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx) -> play_card_target {
                auto &&handler = effect_cast<T>(effect.target_value);
                if constexpr (requires { handler.random_target(origin_card, origin, effect, ctx); }) {
                    return play_card_target{value_type{handler.random_target(origin_card, origin, effect, ctx)}};
                } else {
                    return play_card_target{value_type{random_element(handler.possible_targets(origin_card, origin, effect, ctx), origin->m_game->bot_rng)}};
                }
            },

            .get_error = [](card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) -> game_string {
                return effect_cast<T>(effect.target_value).get_error(origin_card, origin, effect, ctx, target.get<value_type>());
            },

            .on_prompt = [](card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) -> prompt_string {
                return effect_cast<T>(effect.target_value).on_prompt(origin_card, origin, effect, ctx, target.get<value_type>());
            },

            .add_context = [](card_ptr origin_card, player_ptr origin, const effect_holder &effect, effect_context &ctx, const play_card_target &target) {
                effect_cast<T>(effect.target_value).add_context(origin_card, origin, effect, ctx, target.get<value_type>());
            },

            .on_play = [](card_ptr origin_card, player_ptr origin, const effect_holder &effect, const effect_context &ctx, const play_card_target &target) {
                effect_cast<T>(effect.target_value).on_play(origin_card, origin, effect, ctx, target.get<value_type>());
            }
        };
    }

    #ifdef BUILD_TARGETING_VTABLE
    #undef BUILD_TARGETING_VTABLE
    #endif

    #define BUILD_TARGETING_VTABLE(name, type) template<> const targeting_vtable targeting_vtable_map<#name>::value = build_targeting_vtable<type>(#name);
    #define TARGET_VALUE(name) targeting_vtable_map<#name>::type

    #undef TRY_RETURN

}

#endif