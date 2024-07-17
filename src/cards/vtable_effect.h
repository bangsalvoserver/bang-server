#ifndef __VTABLE_EFFECT_H__
#define __VTABLE_EFFECT_H__

#include "card_defs.h"

#include "utils/fixed_string.h"

#include "game/filters_simple.h"

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
    
    inline game_string effect_holder::get_error(card *origin_card, player *origin, const effect_context &ctx) const {
        return type->get_error(effect_value, origin_card, origin, ctx);
    }

    inline game_string effect_holder::get_error(card *origin_card, player *origin, player *target, const effect_context &ctx) const {
        return type->get_error_player(effect_value, origin_card, origin, target, ctx);
    }

    inline game_string effect_holder::get_error(card *origin_card, player *origin, card *target, const effect_context &ctx) const {
        return type->get_error_card(effect_value, origin_card, origin, target, ctx);
    }

    inline game_string effect_holder::on_prompt(card *origin_card, player *origin, const effect_context &ctx) const {
        return type->on_prompt(effect_value, origin_card, origin, ctx);
    }

    inline game_string effect_holder::on_prompt(card *origin_card, player *origin, player *target, const effect_context &ctx) const {
        return type->on_prompt_player(effect_value, origin_card, origin, target, ctx);
    }

    inline game_string effect_holder::on_prompt(card *origin_card, player *origin, card *target, const effect_context &ctx) const {
        return type->on_prompt_card(effect_value, origin_card, origin, target, ctx);
    }

    inline void effect_holder::add_context(card *origin_card, player *origin, effect_context &ctx) const {
        type->add_context(effect_value, origin_card, origin, ctx);
    }

    inline void effect_holder::add_context(card *origin_card, player *origin, player *target, effect_context &ctx) const {
        type->add_context_player(effect_value, origin_card, origin, target, ctx);
    }

    inline void effect_holder::add_context(card *origin_card, player *origin, card *target, effect_context &ctx) const {
        type->add_context_card(effect_value, origin_card, origin, target, ctx);
    }

    inline void effect_holder::on_play(card *origin_card, player *origin, effect_flags flags, const effect_context &ctx) const {
        type->on_play(effect_value, origin_card, origin, flags, ctx);
    }

    inline void effect_holder::on_play(card *origin_card, player *origin, player *target, effect_flags flags, const effect_context &ctx) const {
        type->on_play_player(effect_value, origin_card, origin, target, flags, ctx);
    }

    inline void effect_holder::on_play(card *origin_card, player *origin, card *target, effect_flags flags, const effect_context &ctx) const {
        type->on_play_card(effect_value, origin_card, origin, target, flags, ctx);
    }

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
                    if (filters::is_player_bot(origin) && !value.on_check_target(origin_card, origin)) {
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
                    if (filters::is_player_bot(origin) && !value.on_check_target(origin_card, origin, target)) {
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

            .on_play_player = [](int effect_value, card *origin_card, player *origin, player *target, effect_flags flags, const effect_context &ctx) {
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
                    if (filters::is_player_bot(origin) && !value.on_check_target(origin_card, origin, target)) {
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
    
    template<utils::fixed_string Name> struct effect_vtable_map;

    #define DEFINE_EFFECT(name, type) \
        template<> struct effect_vtable_map<#name> { \
            static constexpr effect_vtable value = build_effect_vtable<type>(#name); \
        };
    
    #define GET_EFFECT(name) (&effect_vtable_map<#name>::value)
}

#endif