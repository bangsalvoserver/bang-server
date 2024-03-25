#ifndef __EFFECT_VTABLE_H__
#define __EFFECT_VTABLE_H__

#include "utils/tstring.h"

#include "mth_unwrapper.h"

namespace banggame {
    struct mth_vtable {
        std::string_view name;
        game_string (*get_error)(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx);
        game_string (*on_prompt)(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx);
        void (*on_play)(card *origin_card, player *origin, const effect_target_list &targets, const effect_context &ctx);
    };

    template<utils::tstring Name, typename T>
    constexpr mth_vtable build_mth_vtable() {
        return {
            .name = std::string_view(Name),

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
                    if (player_is_bot(origin) && !mth_unwrapper{&T::on_check_target}(origin_card, origin, targets, ctx)) {
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
            static constexpr mth_vtable value = build_mth_vtable<#name, type>(); \
        };
    
    #define GET_MTH(name) (&mth_vtable_map<#name>::value)

    DEFINE_MTH(none, void)
}

#endif