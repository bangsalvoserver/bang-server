#include "holders.h"

#include <stdexcept>

#include "cards/effects.h"

#include "game.h"
#include "mth_unwrapper.h"

namespace banggame {

    template<card_expansion_type E>
    inline void do_apply_ruleset(game *game, enums::enum_tag_t<E>) {
        if constexpr (enums::value_with_type<E>) {
            if (bool(game->m_options.expansions & E)) {
                using type = enums::enum_type_t<E>;
                type{}.on_apply(game);
            }
        }
    }

    void game::apply_rulesets() {
        [&]<card_expansion_type ... Es>(enums::enum_sequence<Es ...>) {
            (do_apply_ruleset(this, enums::enum_tag<Es>), ...);
        }(enums::make_enum_sequence<card_expansion_type>());
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

    verify_result effect_holder::verify(card *origin_card, player *origin, const target_variant &target) const {
        return visit_effect([=](auto &&value) -> verify_result {
            switch (target.index()) {
            case 0:
                if constexpr (requires { value.verify(origin_card, origin); }) {
                    return value.verify(origin_card, origin);
                } else if constexpr (requires { value.can_respond(origin_card, origin); }) {
                    if (!value.can_respond(origin_card, origin)) {
                        return "ERROR_INVALID_RESPONSE";
                    }
                }
                break;
            case 1:
                if constexpr (requires (player *target_player) { value.verify(origin_card, origin, target_player); }) {
                    return value.verify(origin_card, origin, std::get<player *>(target));
                }
                break;
            case 2:
                if constexpr (requires (card *target_card) { value.verify(origin_card, origin, target_card); }) {
                    return value.verify(origin_card, origin, std::get<card *>(target));
                }
            }
            return {};
        }, *this);
    }

    game_string effect_holder::on_prompt(card *origin_card, player *origin, const target_variant &target) const {
        return visit_effect([=](auto &&value) -> game_string {
            switch (target.index()) {
            case 0:
                if constexpr (requires { value.on_check_target(origin_card, origin); }) {
                    if (origin->is_bot() && !value.on_check_target(origin_card, origin)) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires { value.on_prompt(origin_card, origin); }) {
                    return value.on_prompt(origin_card, origin);
                }
                break;
            case 1:
                if constexpr (requires (player *target_player) { value.on_check_target(origin_card, origin, target_player); }) {
                    if (origin->is_bot() && !value.on_check_target(origin_card, origin, std::get<player *>(target))) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires (player *target_player) { value.on_prompt(origin_card, origin, target_player); }) {
                    return value.on_prompt(origin_card, origin, std::get<player *>(target));
                }
                break;
            case 2:
                if constexpr (requires (card *target_card) { value.on_check_target(origin_card, origin, target_card); }) {
                    if (origin->is_bot() && !value.on_check_target(origin_card, origin, std::get<card *>(target))) {
                        return "BOT_BAD_PLAY";
                    }
                }
                if constexpr (requires (card *target_card) { value.on_prompt(origin_card, origin, target_card); }) {
                    return value.on_prompt(origin_card, origin, std::get<card *>(target));
                }
            }
            return {};
        }, *this);
    }

    void effect_holder::on_play(card *origin_card, player *origin, const target_variant &target, effect_flags flags) const {
        visit_effect([=](auto &&value) {
            switch (target.index()) {
            case 0:
                if constexpr (requires { value.on_play(origin_card, origin, flags); }) {
                    value.on_play(origin_card, origin, flags);
                } else if constexpr (requires { value.on_play(origin_card, origin); }) {
                    value.on_play(origin_card, origin);
                }
                break;
            case 1:
                if constexpr (requires (player *target_player) { value.on_play(origin_card, origin, target_player, flags); }) {
                    value.on_play(origin_card, origin, std::get<player *>(target), flags);
                } else if constexpr (requires (player *target_player) { value.on_play(origin_card, origin, target_player); }) {
                    value.on_play(origin_card, origin, std::get<player *>(target));
                }
                break;
            case 2:
                if constexpr (requires (card *target_card) { value.on_play(origin_card, origin, target_card, flags); }) {
                    value.on_play(origin_card, origin, std::get<card *>(target), flags);
                } else if constexpr (requires (card *target_card) { value.on_play(origin_card, origin, target_card); }) {
                    value.on_play(origin_card, origin, std::get<card *>(target));
                }
            }
        }, *this);
    }

    game_string equip_holder::on_prompt(player *origin, card *target_card, player *target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.on_check_target(target_card, origin, target); }) {
                if (origin->is_bot() && !value.on_check_target(target_card, origin, target)) {
                    return "BOT_BAD_PLAY";
                }
            }
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

    game_string modifier_holder::on_prompt(card *origin_card, player *origin, card *playing_card) const {
        return enums::visit_enum([&]<card_modifier_type E>(enums::enum_tag_t<E>) -> game_string {
            if constexpr (enums::value_with_type<E>) {
                enums::enum_type_t<E> handler;
                if constexpr (requires { handler.on_prompt(origin_card, origin, playing_card); }) {
                    return handler.on_prompt(origin_card, origin, playing_card);
                }
            }
            return {};
        }, type);
    }

    verify_result modifier_holder::verify(card *origin_card, player *origin, card *playing_card) const {
        return enums::visit_enum([&]<card_modifier_type E>(enums::enum_tag_t<E>) -> verify_result {
            if constexpr (enums::value_with_type<E>) {
                enums::enum_type_t<E> handler;
                if constexpr (requires { handler.verify(origin_card, origin, playing_card); }) {
                    return handler.verify(origin_card, origin, playing_card);
                }
            }
            return {};
        }, type);
    }

    verify_result mth_holder::verify(card *origin_card, player *origin, const target_list &targets) const {
        return enums::visit_enum([&]<mth_type E>(enums::enum_tag_t<E>) -> verify_result {
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
                if constexpr (requires { mth_unwrapper{&handler_type::on_check_target}; }) {
                    if (origin->is_bot() && !mth_unwrapper{&handler_type::on_check_target}(origin_card, origin, targets)) {
                        return "BOT_BAD_PLAY";
                    }
                }
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