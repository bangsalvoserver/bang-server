#ifndef __EFFECT_VTABLE_H__
#define __EFFECT_VTABLE_H__

#include "utils/tstring.h"

#include "cards/card_data.h"

namespace banggame {

    struct effect_vtable {
        std::string_view name;
        
        game_string (*get_error)(int effect_value, card *origin_card, player *origin, const effect_context &ctx);
        game_string (*on_prompt)(int effect_value, card *origin_card, player *origin, const effect_context &ctx);
        void (*add_context)(int effect_value, card *origin_card, player *origin, effect_context &ctx);
        void (*on_play)(int effect_value, card *origin_card, player *origin, effect_flags flags, const effect_context &ctx);

        game_string (*get_error_player)(int effect_value, card *origin_card, player *origin, player *target, const effect_context &ctx);
        game_string (*on_prompt_player)(int effect_value, card *origin_card, player *origin, player *target, const effect_context &ctx);
        void (*add_context_player)(int effect_value, card *origin_card, player *origin, player *target, effect_context &ctx);
        void (*on_play_player)(int effect_value, card *origin_card, player *origin, player *target, effect_flags flags, const effect_context &ctx);
        
        game_string (*get_error_card)(int effect_value, card *origin_card, player *origin, card *target, const effect_context &ctx);
        game_string (*on_prompt_card)(int effect_value, card *origin_card, player *origin, card *target, const effect_context &ctx);
        void (*add_context_card)(int effect_value, card *origin_card, player *origin, card *target, effect_context &ctx);
        void (*on_play_card)(int effect_value, card *origin_card, player *origin, card *target, effect_flags flags, const effect_context &ctx);
    };

    template<typename T>
    inline auto build_effect(int effect_value) {
        if constexpr (requires { T{effect_value}; }) {
            return T{effect_value};
        } else {
            return T{};
        }
    }
    
    template<utils::tstring Name, typename T>
    constexpr effect_vtable build_effect_vtable() {
        return {
            .name = std::string_view(Name),

            .get_error = [](int effect_value, card *origin_card, player *origin, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, ctx); }) {
                    return value.get_error(origin_card, origin, ctx);
                } else if constexpr (requires (T value) { value.get_error(origin_card, origin); }) {
                    return value.get_error(origin_card, origin);
                } else if constexpr (requires (T value) { value.can_play(origin_card, origin, ctx); }) {
                    if (!value.can_play(origin_card, origin, ctx)) {
                        return "ERROR_INVALID_ACTION";
                    }
                } else if constexpr (requires (T value) { value.can_play(origin_card, origin); }) {
                    if (!value.can_play(origin_card, origin)) {
                        return "ERROR_INVALID_ACTION";
                    }
                }
                return {};
            },

            .on_prompt = [](int effect_value, card *origin_card, player *origin, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_check_target(origin_card, origin); }) {
                    if (player_is_bot(origin) && !value.on_check_target(origin_card, origin)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { value.on_prompt(origin_card, origin, ctx); }) {
                    return value.on_prompt(origin_card, origin, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin); }) {
                    return value.on_prompt(origin_card, origin);
                }
                return {};
            },

            .add_context = [](int effect_value, card *origin_card, player *origin, effect_context &ctx){
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, ctx); }) {
                    value.add_context(origin_card, origin, ctx);
                }
            },

            .on_play = [](int effect_value, card *origin_card, player *origin, effect_flags flags, const effect_context &ctx) {
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

            .get_error_player = [](int effect_value, card *origin_card, player *origin, player *target, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, target, ctx); }) {
                    return value.get_error(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target); }) {
                    return value.get_error(origin_card, origin, target);
                }
                return {};
            },
            
            .on_prompt_player = [](int effect_value, card *origin_card, player *origin, player *target, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_check_target(origin_card, origin, target); }) {
                    if (player_is_bot(origin) && !value.on_check_target(origin_card, origin, target)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { value.on_prompt(origin_card, origin, target, ctx); }) {
                    return value.on_prompt(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .add_context_player = [](int effect_value, card *origin_card, player *origin, player *target, effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, target, ctx); }) {
                    value.add_context(origin_card, origin, target, ctx);
                }
            },

            .get_error_card = [](int effect_value, card *origin_card, player *origin, card *target, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.get_error(origin_card, origin, target, ctx); }) {
                    return value.get_error(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.get_error(origin_card, origin, target); }) {
                    return value.get_error(origin_card, origin, target);
                }
                return {};
            },

            .on_prompt_card = [](int effect_value, card *origin_card, player *origin, card *target, const effect_context &ctx) -> game_string {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.on_check_target(origin_card, origin, target); }) {
                    if (player_is_bot(origin) && !value.on_check_target(origin_card, origin, target)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { value.on_prompt(origin_card, origin, target, ctx); }) {
                    return value.on_prompt(origin_card, origin, target, ctx);
                } else if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                    return value.on_prompt(origin_card, origin, target);
                }
                return {};
            },

            .add_context_card = [](int effect_value, card *origin_card, player *origin, card *target, effect_context &ctx) {
                T value = build_effect<T>(effect_value);
                if constexpr (requires { value.add_context(origin_card, origin, target, ctx); }) {
                    value.add_context(origin_card, origin, target, ctx);
                }
            },

            .on_play_card = [](int effect_value, card *origin_card, player *origin, card *target, effect_flags flags, const effect_context &ctx) {
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
    
    template<utils::tstring Name> struct effect_vtable_map;

    #define DEFINE_EFFECT(name, type) \
        template<> struct effect_vtable_map<#name> { \
            static constexpr effect_vtable value = build_effect_vtable<#name, type>(); \
        };
    
    #define GET_EFFECT(name) (&effect_vtable_map<#name>::value)

    struct effect_none {};
    DEFINE_EFFECT(none, effect_none)
}

#endif