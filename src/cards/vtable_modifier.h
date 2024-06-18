#ifndef __VTABLE_MODIFIER_H__
#define __VTABLE_MODIFIER_H__

#include "utils/tstring.h"

#include "game/filters_simple.h"

namespace banggame {

    struct modifier_vtable {
        std::string_view name;

        void (*add_context)(card *origin_card, player *origin, effect_context &ctx);
        game_string (*get_error)(card *origin_card, player *origin, card *target_card, const effect_context &ctx);
        game_string (*on_prompt)(card *origin_card, player *origin, card *playing_card, const effect_context &ctx);
    };

    template<typename T>
    constexpr modifier_vtable build_modifier_vtable(std::string_view name) {
        return {
            .name = name,

            .add_context = [](card *origin_card, player *origin, effect_context &ctx) {
                if constexpr (requires (T value) { value.add_context(origin_card, origin, ctx); }) {
                    T{}.add_context(origin_card, origin, ctx);
                }
            },

            .get_error = [](card *origin_card, player *origin, card *target_card, const effect_context &ctx) -> game_string {
                if (card *equip_card = filters::get_equip_card(target_card, ctx)) {
                    target_card = equip_card;
                    if constexpr (requires (T handler) { handler.valid_with_equip(origin_card, origin, target_card); }) {
                        if (T{}.valid_with_equip(origin_card, origin, target_card)) {
                            return {};
                        } else {
                            return {"ERROR_CANT_PLAY_WHILE_EQUIPPING", origin_card, target_card};
                        }
                    }
                } else if (filters::is_modifier_card(origin, target_card)) {
                    if constexpr (requires (T handler) { handler.valid_with_modifier(origin_card, origin, target_card); }) {
                        if (T{}.valid_with_modifier(origin_card, origin, target_card)) {
                            return {};
                        } else {
                            return {"ERROR_NOT_ALLOWED_WITH_MODIFIER", origin_card, target_card};
                        }
                    }
                }
                if constexpr (requires (T handler) { handler.valid_with_card(origin_card, origin, target_card); }) {
                    if (!T{}.valid_with_card(origin_card, origin, target_card)) {
                        return {"ERROR_NOT_ALLOWED_WITH_CARD", origin_card, target_card};
                    }
                }
                if constexpr (requires (T handler) { handler.get_error(origin_card, origin, target_card, ctx); }) {
                    return T{}.get_error(origin_card, origin, target_card, ctx);
                } else if constexpr (requires (T handler) { handler.get_error(origin_card, origin, target_card); }) {
                    return T{}.get_error(origin_card, origin, target_card);
                }
                return {};
            },

            .on_prompt = [](card *origin_card, player *origin, card *playing_card, const effect_context &ctx) -> game_string {
                if constexpr (requires (T handler) { handler.on_prompt(origin_card, origin, playing_card, ctx); }) {
                    return T{}.on_prompt(origin_card, origin, playing_card);
                } else if constexpr (requires (T handler) { handler.on_prompt(origin_card, origin, playing_card); }) {
                    return T{}.on_prompt(origin_card, origin, playing_card);
                }
                return {};
            }
        };
    };

    template<utils::tstring Name> struct modifier_vtable_map;

    #define DEFINE_MODIFIER(name, type) \
        template<> struct modifier_vtable_map<#name> { \
            static constexpr modifier_vtable value = build_modifier_vtable<type>(#name); \
        };
    
    #define GET_MODIFIER(name) (&modifier_vtable_map<#name>::value)
}

#endif