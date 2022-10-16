#include "holders.h"

#include "formatter.h"

#include <stdexcept>

#include "effects/effects.h"
#include "effects/scenarios.h"

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

    game_string effect_holder::verify(card *origin_card, player *origin) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.verify(origin_card, origin); }) {
                return value.verify(origin_card, origin);
            } else if constexpr (requires { value.can_respond(origin_card, origin); }) {
                if (!value.can_respond(origin_card, origin)) {
                    return "ERROR_INVALID_RESPONSE";
                }
            }
            return {};
        }, *this);
    }

    game_string effect_holder::verify(card *origin_card, player *origin, player *target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.verify(origin_card, origin, target); }) {
                return value.verify(origin_card, origin, target);
            }
            return {};
        }, *this);
    }

    game_string effect_holder::verify(card *origin_card, player *origin, card *target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.verify(origin_card, origin, target); }) {
                return value.verify(origin_card, origin, target);
            }
            return {};
        }, *this);
    }

    game_string effect_holder::on_prompt(card *origin_card, player *origin) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.on_prompt(origin_card, origin); }) {
                return value.on_prompt(origin_card, origin);
            } else {
                return {};
            }
        }, *this);
    }

    game_string effect_holder::on_prompt(card *origin_card, player *origin, player *target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                return value.on_prompt(origin_card, origin, target);
            } else {
                return {};
            }
        }, *this);
    }

    game_string effect_holder::on_prompt(card *origin_card, player *origin, card *target) const {
        return visit_effect([=](auto &&value) -> game_string {
            if constexpr (requires { value.on_prompt(origin_card, origin, target); }) {
                return value.on_prompt(origin_card, origin, target);
            } else {
                return {};
            }
        }, *this);
    }

    void effect_holder::on_play(card *origin_card, player *origin, effect_flags flags) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_play(origin_card, origin, flags); }) {
                value.on_play(origin_card, origin, flags);
            } else if constexpr (requires { value.on_play(origin_card, origin); }) {
                value.on_play(origin_card, origin);
            } else {
                throw std::runtime_error("missing on_play(origin_card, origin)");
            }
        }, *this);
    }

    void effect_holder::on_play(card *origin_card, player *origin, player *target, effect_flags flags) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_play(origin_card, origin, target, flags); }) {
                value.on_play(origin_card, origin, target, flags);
            } else if constexpr (requires {value.on_play(origin_card, origin, target); }) {
                value.on_play(origin_card, origin, target);
            } else {
                throw std::runtime_error("missing on_play(origin_card, origin, target)");
            }
        }, *this);
    }

    void effect_holder::on_play(card *origin_card, player *origin, card *target, effect_flags flags) const {
        visit_effect([=](auto &&value) {
            if constexpr (requires { value.on_play(origin_card, origin, target, flags); }) {
                value.on_play(origin_card, origin, target, flags);
            } else if constexpr (requires { value.on_play(origin_card, origin, target); }) {
                value.on_play(origin_card, origin, target);
            } else {
                throw std::runtime_error("missing on_play(origin_card, origin, target_card)");
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

    bool request_base::can_respond(player *target, card *target_card) const {
        using namespace enums::flag_operators;

        const bool is_response = !bool(flags & effect_flags::force_play);
        return !target->m_game->is_disabled(target_card) && target->is_possible_to_play(target_card, is_response);
    }

    void request_base::on_pick(pocket_type pocket, player *target, card *target_card) {
        throw std::runtime_error("missing on_pick(pocket, target, target_card)");
    }

    void timer_request::tick() {
        if (awaiting_confirms.empty() && duration != ticks{0} && --duration == ticks{0}) {
            auto copy = shared_from_this();
            target->m_game->pop_request();
            on_finished();
            target->m_game->update_request();
        } else if (auto_confirm_timer != ticks{0} && --auto_confirm_timer == ticks{0}) {
            awaiting_confirms.clear();
        }
    }

    void timer_request::add_pending_confirm(player *p) {
        awaiting_confirms.push_back(p);
    }

    void timer_request::confirm_player(player *p) {
        auto it = std::ranges::find(awaiting_confirms, p);
        if (it != awaiting_confirms.end()) {
            awaiting_confirms.erase(it);
        }
    }

}