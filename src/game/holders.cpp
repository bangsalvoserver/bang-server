#include "holders.h"

#include <stdexcept>

#include "cards/effects.h"

#include "game.h"
#include "mth_unwrapper.h"

namespace banggame {

    void apply_ruleset(game *game, card_expansion_type value) {
        enums::visit_enum([&]<card_expansion_type E>(enums::enum_tag_t<E>) {
            if constexpr (enums::value_with_type<E>) {
                using type = enums::enum_type_t<E>;
                type{}.on_apply(game);
            }
        }, value);
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

    game_string effect_holder::verify(card *origin_card, player *origin, const target_variant &target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if (std::holds_alternative<std::monostate>(target)) {
                if constexpr (requires { value.verify(origin_card, origin); }) {
                    return value.verify(origin_card, origin);
                } else if constexpr (requires { value.can_respond(origin_card, origin); }) {
                    if (!value.can_respond(origin_card, origin)) {
                        return "ERROR_INVALID_RESPONSE";
                    }
                }
            } else if (player * const *target_player = std::get_if<player *>(&target)) {
                if constexpr (requires { value.verify(origin_card, origin, *target_player); }) {
                    return value.verify(origin_card, origin, *target_player);
                }
            } else if (card * const *target_card = std::get_if<card *>(&target)) {
                if constexpr (requires { value.verify(origin_card, origin, *target_card); }) {
                    return value.verify(origin_card, origin, *target_card);
                }
            }
            return {};
        }, *this);
    }

    game_string effect_holder::on_prompt(card *origin_card, player *origin, const target_variant &target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if (std::holds_alternative<std::monostate>(target)) {
                if constexpr (requires { value.on_prompt(origin_card, origin); }) {
                    return value.on_prompt(origin_card, origin);
                }
            } else if (player * const *target_player = std::get_if<player *>(&target)) {
                if constexpr (requires { value.on_prompt(origin_card, origin, *target_player); }) {
                    return value.on_prompt(origin_card, origin, *target_player);
                }
            } else if (card * const *target_card = std::get_if<card *>(&target)) {
                if constexpr (requires { value.on_prompt(origin_card, origin, *target_card); }) {
                    return value.on_prompt(origin_card, origin, *target_card);
                }
            }
            return {};
        }, *this);
    }

    void effect_holder::on_play(card *origin_card, player *origin, const target_variant &target, effect_flags flags) const {
        visit_effect([=](auto &&value) {
            if (std::holds_alternative<std::monostate>(target)) {
                if constexpr (requires { value.on_play(origin_card, origin, flags); }) {
                    value.on_play(origin_card, origin, flags);
                } else if constexpr (requires { value.on_play(origin_card, origin); }) {
                    value.on_play(origin_card, origin);
                }
            } else if (player * const *target_player = std::get_if<player *>(&target)) {
                if constexpr (requires { value.on_play(origin_card, origin, *target_player, flags); }) {
                    value.on_play(origin_card, origin, *target_player, flags);
                } else if constexpr (requires {value.on_play(origin_card, origin, *target_player); }) {
                    value.on_play(origin_card, origin, *target_player);
                }
            } else if (card * const *target_card = std::get_if<card *>(&target)) {
                if constexpr (requires { value.on_play(origin_card, origin, *target_card, flags); }) {
                    value.on_play(origin_card, origin, *target_card, flags);
                } else if constexpr (requires { value.on_play(origin_card, origin, *target_card); }) {
                    value.on_play(origin_card, origin, *target_card);
                }
            }
        }, *this);
    }

    game_string equip_holder::on_prompt(player *origin, card *target_card, player *target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.on_prompt(origin, target_card, target); }) {
                return value.on_prompt(origin, target_card, target);
            }
            return {};
        }, *this);
    }

    void equip_holder::on_equip(card *target_card, player *target) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_equip(target_card, target); }) {
                value.on_equip(target_card, target);
            }
        }, *this);
    }

    void equip_holder::on_enable(card *target_card, player *target) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_enable(target_card, target); }) {
                value.on_enable(target_card, target);
            }
        }, *this);
    }

    void equip_holder::on_disable(card *target_card, player *target) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_disable(target_card, target); }) {
                value.on_disable(target_card, target);
            }
        }, *this);
    }

    void equip_holder::on_unequip(card *target_card, player *target) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_unequip(target_card, target); }) {
                value.on_unequip(target_card, target);
            }
        }, *this);
    }

    game_string mth_holder::verify(card *origin_card, player *origin, const target_list &targets) const {
        return enums::visit_enum([&]<mth_type E>(enums::enum_tag_t<E>) -> game_string {
            if constexpr (enums::value_with_type<E>) {
                using handler_type = enums::enum_type_t<E>;
                if constexpr (requires { mth_unwrapper{&handler_type::verify}; }) {
                    return mth_unwrapper{&handler_type::verify}(origin_card, origin, targets);
                }
            }
            return {};
        }, type);
    }

    game_string mth_holder::on_prompt(card *origin_card, player *origin, const target_list &targets) const {
        return enums::visit_enum([&]<mth_type E>(enums::enum_tag_t<E>) -> game_string {
            if constexpr (enums::value_with_type<E>) {
                using handler_type = enums::enum_type_t<E>;
                if constexpr (requires { mth_unwrapper{&handler_type::on_prompt}; }) {
                    return mth_unwrapper{&handler_type::on_prompt}(origin_card, origin, targets);
                }
            }
            return {};
        }, type);
    }
    
    void mth_holder::on_play(card *origin_card, player *origin, const target_list &targets) const {
        enums::visit_enum([&]<mth_type E>(enums::enum_tag_t<E>) {
            if constexpr (enums::value_with_type<E>) {
                using handler_type = enums::enum_type_t<E>;
                return mth_unwrapper{&handler_type::on_play}(origin_card, origin, targets);
            }
        }, type);
    }

}