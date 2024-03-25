#include "holders.h"

#include <stdexcept>

#include "effects.h"
#include "effect_enums.h"
#include "mth_unwrapper.h"
#include "filters.h"

#include "game/game.h"

namespace banggame {

    template<expansion_type E>
    inline void do_apply_ruleset(game *game, enums::enum_tag_t<E>) {
        if constexpr (enums::value_with_type<E>) {
            if (bool(game->m_options.expansions & E)) {
                using type = enums::enum_type_t<E>;
                type{}.on_apply(game);
            }
        }
    }

    void apply_rulesets(game *game) {
        [&]<expansion_type ... Es>(enums::enum_sequence<Es ...>) {
            (do_apply_ruleset(game, enums::enum_tag<Es>), ...);
        }(enums::make_enum_sequence<expansion_type>());
    }

    template<typename Holder, typename Function>
    static auto visit_effect(Function &&fun, Holder &holder) {
        return enums::visit_enum([&]<decltype(Holder::type) E>(enums::enum_tag_t<E>) {
            if constexpr (enums::value_with_type<E>) {
                using type = enums::enum_type_t<E>;
                if constexpr (requires { type{holder.effect_value}; }) {
                    return fun(type{holder.effect_value});
                } else {
                    return fun(type{});
                }
            } else {
                return fun(std::monostate{});
            }
        }, holder.type);
    }

    game_string equip_holder::on_prompt(card *origin_card, player *origin, player *target) const {
        return visit_effect([&](auto &&value) -> game_string {
            if constexpr (requires { value.on_check_target(origin_card, origin, target); }) {
                if (origin->is_bot() && !value.on_check_target(origin_card, origin, target)) {
                    return "BOT_BAD_PLAY";
                }
            }
            if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                return value.on_prompt(origin_card, origin, target);
            }
            return {};
        }, *this);
    }

    void equip_holder::on_enable(card *target_card, player *target) const {
        visit_effect([&](auto &&value) {
            if constexpr (requires { value.on_enable(target_card, target); }) {
                value.on_enable(target_card, target);
            }
        }, *this);
    }

    void equip_holder::on_disable(card *target_card, player *target) const {
        visit_effect([&](auto &&value) {
            if constexpr (requires { value.on_disable(target_card, target); }) {
                value.on_disable(target_card, target);
            }
        }, *this);
    }

    template<equip_type E> static constexpr bool is_nodisable_v = requires {
        typename enums::enum_type_t<E>::nodisable;
    };

    bool equip_holder::is_nodisable() const {
        static constexpr auto nodisable_table = []<equip_type ... Es>(enums::enum_sequence<Es ...>) {
            return std::array { is_nodisable_v<Es> ... };
        }(enums::make_enum_sequence<equip_type>());
        return nodisable_table[enums::indexof(type)];
    }

}